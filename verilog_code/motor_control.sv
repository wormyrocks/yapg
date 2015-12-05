///////////////////////////////////////////////////////////
// motor_control                                         //
//                                                       //
//This is our top level module. It reads the motor speed //
//from the encoder, passes that through a control loop,  //
//and then outputs the result as a PWM signal            //
//                                                       //
//CONTROL LOOP IS NOT IMPLEMENTED YET                    //
///////////////////////////////////////////////////////////


module motor_control(input logic encoder, clk, reset,
                     output logic signal);
  //Read the data from the encoder
  logic [31:0] period;
  read_encoder read_encoder(encoder,clk,reset,period);

  //Run the data from the encoder through a control loop to 
  //determine the output of the system to the motor.
  logic [31:0] desired_period;
  assign desired_period = 32'h0004E20;
  logic[9:0] duty_cycle;
  control_loop control_loop(period,desired_period,duty_cycle);
  
  //Set up the 10-bit PWM at 4.88kHz to control the motor
  //based on the output of the control loop.
  PWM #(10,3) PWM(duty_cycle,clk,reset,signal);	
  
endmodule



///////////////////////////////////////////////////////////
// read_encoder                                          //
//                                                       //
//This module reads the encoder signal from a motor, and //
//updates the most recent period in clock cycles         //
///////////////////////////////////////////////////////////
module read_encoder(input logic encoder, clk, reset,
                    output logic [31:0] period);
  //save the current and previous values of the encoder
  //so that we can catch the rising edge
  logic prev_encoder, synch_encoder;
  logic [31:0] counter;
  always_ff @(posedge clk, posedge reset) begin
    if (reset)
	   period <= 32'h7FFF_FFFF;
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
// control_loop                                          //
//                                                       //
//This module implements our motor control loop.         //
//                                                       //
//TODO: WRITE DETAILS                                    //
///////////////////////////////////////////////////////////
module control_loop(input logic [31:0] period, desired_period,
                    output logic [9:0] duty_cycle);
  logic [31:0] err;
  assign err = desired_period - period;
  assign duty_cycle = err[31] ? 10'b_11_1111_1111 : 10'b0;
						  
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
