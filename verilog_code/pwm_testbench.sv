module pwm_testbench();
  //initialize necessary signals
  logic clk, reset;
  logic [3:0] duty_cycle1;
  logic signal1;
  
  logic [3:0] duty_cycle2;
  assign duty_cycle2 = 4'b0100;
  logic signal2;
  
  logic [3:0] duty_cycle3;
  assign duty_cycle3 = 4'b1100;
  logic signal3;
  
  // create our controller module
  PWM #(4,2) DUT1(duty_cycle1, clk, reset, signal1);
  PWM #(4,2) DUT2(duty_cycle2, clk, reset, signal2);
  PWM #(4,2) DUT3(duty_cycle3, clk, reset, signal3);
  // initialize our clock with a period of 10				
  always begin
    clk = 1'b1; #5; clk = 1'b0; #5;
  end
  
  /////////////////
  // BEGIN TESTS //
  /////////////////

  initial begin
    duty_cycle1 = 4'b1000;
    reset = 1;
	 #17;
	 reset = 0;
	 #3;
	 #2000;
	 duty_cycle1 = 4'b0010;
  end
  
endmodule
