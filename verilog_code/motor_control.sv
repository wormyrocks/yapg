///////////////////////////////////////////////////////////
// motor_control                                         //
//                                                       //
//This is our top level module. It reads the motor speed //
//from the encoder, passes that through a control loop,  //
//and then outputs the result as a PWM signal            //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////


module motor_control(input logic encoder, clk, reset, motor_on,
                     output logic signal,
							output logic [1:0] error_leds);
  //Read the data from the encoder
  logic signed [31:0] period;
  read_encoder read_encoder(encoder,clk,reset,period);

  //Run the data from the encoder through a control loop to 
  //determine the output of the system to the motor.
  logic control_clk;
  clock_divider #(10) control_clk_generator(clk,control_clk);
  logic signed [31:0] desired_period;
  assign desired_period = 32'd20597; //500RPM
  logic[9:0] duty_cycle;
  PI_control_loop control_loop(control_clk,reset,period,desired_period,duty_cycle,error_leds);
  
  logic[9:0] motor_duty_cycle;
  assign motor_duty_cycle = motor_on ? duty_cycle : 10'b0;
  //Set up the 10-bit PWM at 4.88kHz to control the motor
  //based on the output of the control loop.
  PWM #(10,3) PWM(motor_duty_cycle,clk,reset,signal);	
  
endmodule



///////////////////////////////////////////////////////////
// read_encoder                                          //
//                                                       //
//This module reads the encoder signal from a motor, and //
//updates the most recent period in clock cycles         //
///////////////////////////////////////////////////////////
module read_encoder(input logic encoder, clk, reset,
                    output logic signed [31:0] period);
  //save the current and previous values of the encoder
  //so that we can catch the rising edge
  logic prev_encoder, synch_encoder;
  logic [31:0] counter;
  always_ff @(posedge clk, posedge reset) begin
    if (reset)
	   period <= 32'h072F1;
	 else begin
      prev_encoder <= synch_encoder;
      synch_encoder <= encoder;
		
		//reset the counter and set the output period to the count value
      //on every rising edge of the encoder
		if (synch_encoder^prev_encoder) begin
		  period <= counter;
		  counter <= 32'b1;
		end
	   else counter <= counter + 1'b1;
		
	 end
  end
  
endmodule

///////////////////////////////////////////////////////////
// PI_control_loop                                       //
//                                                       //
//This module implements our motor control loop.         //
//We are running a PI control loop to get the motor to   //
//stablize at the desired rotation rate.                 //
//The integral term is needed to remove steady state     //
//error, but only used once we are close to the desired  //
//period.                                                //
///////////////////////////////////////////////////////////
module PI_control_loop(input logic clk, reset,
                       input logic signed [31:0] period, desired_period,
                       output logic [9:0] duty_cycle,
							  output logic [1:0] error);
  logic signed [31:0] err,i,out;
  assign err = period - desired_period;
  
  
  always_ff @(posedge clk, posedge reset) begin
   if (reset) i <= 32'b0;
   else if (i < $signed(32'b0)) i <= 32'b0;
   else if (err < $signed(32'hFFF) && err > $signed(-32'hFFF)) i <= err + i;
  end
  
  assign out = (err >>> 'd3) + (i >>> 'd14);
  always_comb 
    if (out < $signed(32'b0)) begin
	   duty_cycle = 10'b0;
		error = 2'b01;
	 end
	 else if (out > $signed(32'b_11_1000_0000)) begin
	   duty_cycle = 10'b_11_1000_0000;
		error = 2'b10;
	 end
	 else begin
	   duty_cycle = out[9:0];
	   error = 2'b00;
	 end
						  
endmodule


///////////////////////////////////////////////////////////
// PWM                                                   //
//                                                       //
//This module outputs a pulse width modulated signal.    //
//The pulse width is determined by the input duty cycle, //
//and the parameter N is the resolution of the duty      //
//cycle, in bits.                                        //
//The frequency of the PWM signal is dependent on the    //
//input parameter CLK_DIV and the duty cycle resolution. //
//We generate a slow clock based on the CLK_DIV, and     //
//then run the PWM logic off of that new clock.          //
//The frequency of the PWM signal is:                    //
//                  f = f_clk/2^(CLK_DIV + N)            //
///////////////////////////////////////////////////////////
module PWM #(parameter N = 10, CLK_DIV = 32)
            (input  logic [N-1:0] duty_cycle,
             input  logic         clk, reset,
			    output logic         signal);
				 
  //Internal logic we will need
  logic slow_clk; //the slowed down clock for the PWM
  logic [CLK_DIV-1:0] slow_clk_counter; //the counter used to slow down the clock
  logic [N-1:0] counter; //counter for PWM output
  
  //set up the slow clock and reset logic
  always_ff @(posedge clk, posedge reset) begin
    if (reset) begin
	   slow_clk_counter <= 1'b0;
		slow_clk <= 1'b0;
	 end
	 else begin
	   slow_clk_counter <= slow_clk_counter + 1'b1;
		slow_clk <= slow_clk_counter[CLK_DIV-1];
	 end
  end
  
  
  //Set up the counter to count on the slow clock
  always_ff @(posedge slow_clk, posedge reset) begin
    if (reset) counter <= 1'b0;
	 else counter <= counter + 1'b1;
  end
	 
  //set up a new signal that samples the duty cycle at the beginning of each
  //PWM cycle
  logic [N-1:0] sync_duty_cycle;
  always_ff @(posedge slow_clk, posedge reset)
    if (reset) sync_duty_cycle <= duty_cycle;
	 else if (~|counter) sync_duty_cycle <= duty_cycle;
  
  
  //These signals tell us when to raise or lower the output
  logic raise_signal, lower_signal;
  assign lower_signal = ~|(sync_duty_cycle^counter);
  assign raise_signal = ~|counter;
  
  //Set up the output signal based on the lower and raise values
  always_ff @(posedge slow_clk)
    if (lower_signal) signal <= 1'b0;
	 else if (raise_signal) signal <= 1'b1;
	 
endmodule


module clock_divider #(parameter CLK_DIV = 32) 
                      (input logic clk, output logic slow_clk);
  logic [CLK_DIV-1:0] slow_clk_counter; //the counter used to slow down the clock
  
  //set up the slow clock and reset logic
  always_ff @(posedge clk) begin
    slow_clk_counter <= slow_clk_counter + 1'b1;
    slow_clk <= slow_clk_counter[CLK_DIV-1];
  end
endmodule
