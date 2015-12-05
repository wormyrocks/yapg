module encoder_testbench();
  //initialize necessary signals
  logic encoder, clk;
  logic [31:0] period;
  
  // create our read encoder module
  read_encoder DUT(encoder, clk, period);
  
  // initialize our clock with a period of 10				
  always begin
    clk = 1'b1; #5; clk = 1'b0; #5;
  end
  
  /////////////////
  // BEGIN TESTS //
  /////////////////

  initial begin
    #1;
	 
	 //set up synch encoder and prev encoder
    encoder = 1'b1; #5; encoder = 1'b0; #5;
    encoder = 1'b1; #5; encoder = 1'b0; #5;
	 
	 
	 //three cycles
    encoder = 1'b1; #10; encoder = 1'b0; #20;
	 
	 //four cycles
    encoder = 1'b1; #30; encoder = 1'b0; #40;
	 
	 //five cycles
    encoder = 1'b1; #50; encoder = 1'b0; #60;
	 
    //ten cycles
    encoder = 1'b1; #70; encoder = 1'b0; #80;
	 
	 //twenty cycles
    encoder = 1'b1; #90; encoder = 1'b0; #100;
	 
	 //twenty cycles
    encoder = 1'b1; #200; encoder = 1'b0; #100;
	 
	 encoder = 1'b1;
	 
  end
endmodule
