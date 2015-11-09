ipblock instmem(in address : ns(5);
				in wr,rd   : ns(1);
				in idata   : ns(32);
				out odata  : ns(32)) {
	iptype "ram";
	ipparm "size=64";
	ipparm "wl=32";
	ipparm "file=instmem.txt";
}

dp registerfile(in asel 	   : ns(3);
				in bsel 	   : ns(3);
				in storesel    : ns(3);
				in storeenable : ns(1);
				in storedata   : tc(32);
				out a 		   : tc(32);
				out b 		   : tc(32)) {

	reg r1,r2,r3,r4,r5,r6,r7 : tc(32);
	always {

		a = (asel == 0) ? 0 : 
			(asel == 1) ? r1 :
			(asel == 2) ? r2 :
			(asel == 3) ? r3 :
			(asel == 4) ? r4 :
			(asel == 5) ? r5 :
			(asel == 6) ? r6 : r7;

		b = (bsel == 0) ? 0 : 
			(bsel == 1) ? r1 :
			(bsel == 2) ? r2 :
			(bsel == 3) ? r3 :
			(bsel == 4) ? r4 :
			(bsel == 5) ? r5 :
			(bsel == 6) ? r6 : r7;

		
		r1 = (storesel == 1 & storeenable) ? storedata : r1;
		r2 = (storesel == 2 & storeenable) ? storedata : r2;
		r3 = (storesel == 3 & storeenable) ? storedata : r3;
		r4 = (storesel == 4 & storeenable) ? storedata : r4;
		r5 = (storesel == 5 & storeenable) ? storedata : r5;
		r6 = (storesel == 6 & storeenable) ? storedata : r6;
		r7 = (storesel == 7 & storeenable) ? storedata : r7;
	}
}

dp ALU(in select : ns(2);
	   in a  	 : tc(32);
	   in b  	 : tc(32);
	   out r 	 : tc(32);
	   out N 	 : ns(1);
	   out Z 	 : ns(1)) {
	always {
		r = (select == 0) ? 0 : 
			(select == 1) ? a - b : a + b;
		
		N = r[0];
		Z = (r == 0) ? 1 : 0;	
	}
}

dp PC (in pc_select 	  : ns(1);
	   in jump_destination 	  : ns(5);
	   out pc_out : ns(5)) {
	
	reg pcr : ns(5);
	
	always {
		pcr = pc_select ? jump_destination : pcr + 1;
		pc_out = pcr;
	}
}

dp bus_mux(in mssel		 : ns(1);
		   in bus_data 	 : tc(32);
		   in r 		 : tc(32);
		   out storedata : tc(32)) {
	always {
		storedata = mssel ? bus_data : r;
	}
}

dp Controller(in ir 			   : ns(32);
			  in Z  			   : ns(1);
			  in N  			   : ns(1);
			  out alu_select 	   : ns(2);
			  out pc_select  	   : ns(1);
			  out asel 			   : ns(3);
			  out bsel 			   : ns(3);
			  out storesel 		   : ns(3);
			  out storeenable 	   : ns(1);
			  out bus_select 	   : ns(1);
			  out mssel 		   : ns(1);
			  out jump_destination : ns(5)) {
	always {


		storesel = (ir[28:31] == 1 | ir[28:31] == 3 | ir[28:31] == 5) ? ir[25:27] : 0;
		storeenable = (ir[28:31] == 1 | ir[28:31] == 3 | ir[28:31] == 5) ? 1 : 0;
		mssel = ir[28:31] == 1 ? 1 : 0;

		asel = (ir[28:31] == 3 | ir[28:31] == 5) ? ir[22:24] : 0;
		bsel = (ir[28:31] == 0 | ir[28:31] == 3 | ir[28:31] == 5) ? ir[19:21] : 0;

		alu_select = ir[28:31] == 3 ? 2 : ir[28:31] == 5 ? 1 : 0;

		jump_destination = (ir[28:31] == 2 | ir[28:31] == 6) ? ir[14:18] : 0;

		pc_select = (ir[28:31] == 2 | (ir[28:31] == 6 & N == 1)) ? 1 : 0;

		bus_select = ir[28:31] == 0 ? 1 : 0;


		// STORE 0
		// LOAD  1
		// JUMP  2
		// ADD   3
		// ADDI  4
		// SUB   5
		// BNEG  6
	}
}

dp CPU() {
	sig ir : ns(32);
	sig pc_select, storeenable, bus_select, mssel : ns(1);
	reg Z, N : ns(1);
	sig alu_select : ns(2);
	sig asel, bsel, storesel: ns(3);
	sig jump_destination : ns(5);

	sig address : ns(5);
	sig wr, rd : ns(1);
	sig idata, odata : ns(32);

	sig a, b, r : tc(32);

	sig storedata, bus_data : ns(32);

	sig pc_out : ns(5);


	use Controller(ir, Z, N, alu_select, pc_select, asel, bsel, storesel, storeenable, bus_select, mssel, jump_destination);
	use PC(pc_select, jump_destination, pc_out);
	use instmem(address, wr, rd, idata, odata);
	use ALU(alu_select, a, b, r, N, Z);
	use registerfile(asel, bsel, storesel, storeenable, storedata, a, b);
	use bus_mux(mssel, bus_data, r, storedata);

	always{
		$display($dec, "Cycle: ", $cycle, ", PC: ", pc_out, ", ir: ", ir[28:31], " ", ir, ", jump_destination: ", jump_destination, ", r: ", r);

		bus_data = 40;

		wr = 0; rd = 1; idata = 1;
		address = pc_out;
		ir = odata;

	}

}


system sys {
	CPU;
}