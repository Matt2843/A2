$option "vcd"


////////////////////////////
// CPU
////////////////////////////


ipblock instmem(in address : ns(5);
        in wr,rd   : ns(1);
        in idata   : ns(32);
        out odata  : ns(32)) {
  iptype "ram";
  ipparm "size=64";
  ipparm "wl=32";
  ipparm "file=instmem.txt";
}

dp registerfile(in asel      : ns(3);
        in bsel      : ns(3);
        in storesel    : ns(3);
        in storeenable : ns(1);
        in storedata   : tc(32);
        out a        : tc(32);
        out b        : tc(32)) {

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
     in a    : tc(32);
     in b    : tc(32);
     out r   : tc(32);
     out N   : ns(1);
     out Z   : ns(1)) {
  always {
    r = (select == 0) ? 0 : 
      (select == 1) ? a - b : a + b;
    
    N = r[0];
    Z = (r == 0) ? 1 : 0; 
  }
}

dp PC (in pc_select     : ns(1);
     in jump_destination    : ns(5);
     out pc_out : ns(5)) {
  
  reg pcr : ns(5);
  
  always {
    pcr = pc_select ? jump_destination : pcr + 1;
    pc_out = pcr;
  }
}

dp bus_mux(in mssel    : ns(1);
       in M_dataout   : tc(32);
       in r      : tc(32);
       out storedata : tc(32)) {
  always {
    storedata = mssel ? M_dataout : r;
  }
}

dp Controller(in ir          : ns(32);
        in Z           : ns(1);
        in N           : ns(1);
        out alu_select     : ns(2);
        out pc_select      : ns(1);
        out asel         : ns(3);
        out bsel         : ns(3);
        out storesel       : ns(3);
        out storeenable      : ns(1);
        out M_datainrdy     : ns(1);
        out mssel        : ns(1);
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

    M_datainrdy = ir[28:31] == 0 ? 1 : 0;


    // STORE 0
    // LOAD  1
    // JUMP  2
    // ADD   3
    // ADDI  4
    // SUB   5
    // BNEG  6
  }
}

dp CPU(
           out M_datain : ns(32);
           out M_cmd : ns(32);
           out M_datainrdy : ns(1);
           in M_dataout : ns(32);
           in M_dataoutrdy : ns(1)
           //in stall_cpu : ns(1)
       ){

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

          sig storedata : ns(32);

          sig pc_out : ns(5);


       		// Implement your CPU here. 
       		// Please remove the code below (it is only added such that that the platform can run without faults)
          use Controller(ir, Z, N, alu_select, pc_select, asel, bsel, storesel, storeenable, bus_select, mssel, jump_destination);
          use PC(pc_select, jump_destination, pc_out);
          use instmem(address, wr, rd, idata, odata);
          use ALU(alu_select, a, b, r, N, Z);
          use registerfile(asel, bsel, storesel, storeenable, storedata, a, b);
          use bus_mux(mssel, M_dataout, r, storedata);
          //use masterbusinterface()
 
       always{
       		//M_datain = 0;
       		M_cmd = 0;
       		M_datainrdy = 0;

          wr = 0; rd = 1; idata = 1;
          address = pc_out;
          ir = odata;


       }
}

////////////////////////////
// Bus
////////////////////////////

dp bus(
        // CPU interface
        in reqCPU  : ns(1);
        in cmdCPU : ns(32); // What command should be executed at the unit receiving this + ID of destination unit is in 4 MSB
        in dataoutCPU : ns(32); // data from the CPU
        out ackCPU : ns(1); 
                
        out dataRdyCPU : ns(1); // high when unit has data, which is ready for the CPU
        out datainCPU : ns(32); // data to the CPU
        in waitCPU : ns(1); // Remains high until the CPU has read the data
                
        // Data Memory interface
        out reqDataMem : ns(1);
        out cmdDataMem : ns(32);
        out datainDataMem : ns(32); // data to Data memory from CPU
        in ackDataMem : ns(1);
               
        in dataRdyDataMem : ns(1); 
        in dataoutDataMem : ns(32); // data from Data memory to CPU
        out waitDataMem : ns(1);
        
        // Sensor interface
        out reqSensor : ns(1);
        out cmdSensor : ns(32); // only cmd is needed to the sensor
        out NotUsed1 : ns(32); // dataport not used
        in ackSensor : ns(1);
        
        in dataRdySensor : ns(1);
        in dataoutSensor : ns(32);
        out waitSensor : ns(1);
        
        // Output interface
        out reqOutput : ns(1);
        out cmdOutput : ns(32);
        out datainOutput : ns(32); // data to output from CPU
        in ackOutput : ns(1);
               
        in dataRdyOutput : ns(1); 
        in dataoutOutput : ns(32); // data from output to CPU
        out waitOutput : ns(1);
        
        // IDs
        in ID_Sensor : ns(4);
        in ID_DataMem : ns(4);
        in ID_Output : ns(4)
		
        ){
        
        $trace(reqDataMem, "traces/reqDataMem.seq");
        $trace(cmdDataMem, "traces/cmdDataMem.seq");
        $trace(datainDataMem, "traces/datainDataMem.seq");
        $trace(ackDataMem, "traces/ackDataMem.seq");
        $trace(dataRdyDataMem, "traces/dataRdyDataMem.seq");
        $trace(dataoutDataMem, "traces/dataoutDataMem.seq");
        $trace(waitDataMem, "traces/waitDataMem.seq");
        
        $trace(reqSensor, "traces/reqSensor.seq");
        $trace(cmdSensor, "traces/cmdSensor.seq");
        $trace(ackSensor, "traces/ackSensor.seq");
        $trace(dataRdySensor, "traces/dataRdySensor.seq");
        $trace(dataoutSensor, "traces/dataoutSensor.seq");
        $trace(waitSensor, "traces/waitSensor.seq");
        
        $trace(reqOutput, "traces/reqOutput.seq");
        $trace(cmdOutput, "traces/cmdOutput.seq");
        $trace(datainOutput, "traces/datainOutput.seq");
        $trace(ackOutput, "traces/ackOutput.seq");
        $trace(dataRdyOutput, "traces/dataRdyOutput.seq");
        $trace(dataoutOutput, "traces/dataoutOutput.seq");
        $trace(waitOutput, "traces/waitOutput.seq");
        
        $trace(reqCPU, "traces/reqCPU.seq");
        $trace(cmdCPU, "traces/cmdCPU.seq");
        $trace(dataoutCPU, "traces/dataoutCPU.seq");
        $trace(ackCPU, "traces/ackCPU.seq");
        $trace(dataRdyCPU, "traces/dataRdyCPU.seq");
        $trace(datainCPU, "traces/datainCPU.seq");
        $trace(waitCPU, "traces/waitCPU.seq");
        
        always{
          ackCPU = (cmdCPU[28:31] == ID_DataMem & ackDataMem) | (cmdCPU[28:31] == ID_Sensor & ackSensor) | (cmdCPU[28:31] == ID_Output & ackOutput); // "OR" all slaves ack signals here
          dataRdyCPU = dataRdyDataMem | dataRdySensor | dataRdyOutput; // Only one slave can access the bus at any one time (only the slave which responds to the cmd)
          
          reqDataMem = reqCPU;
          cmdDataMem = (reqCPU) ? cmdCPU : 0;
          datainDataMem = (reqCPU) ? dataoutCPU : 0;
          
          reqSensor = reqCPU;
          cmdSensor = (reqCPU) ? cmdCPU : 0;
          waitSensor = waitCPU;
          
          reqOutput = reqCPU;
          cmdOutput = (reqCPU) ? cmdCPU : 0;
          datainOutput = (reqCPU) ? dataoutCPU : 0;
          waitOutput = waitCPU;
                    
          datainCPU = (dataRdyDataMem) ? dataoutDataMem : 
                      (dataRdySensor) ? dataoutSensor : 
                      (dataRdyOutput) ? dataoutOutput :
                      0; // arbitrate here when multiple slaves
          waitDataMem = waitCPU;
          
          NotUsed1 = 0;
        }       
}

dp slavebusinterface(
          // Bus ports
          in bus_req : ns(1);
          in bus_cmd : ns(32);
          in bus_datain : ns(32);
          out bus_ack : ns(1);
          
          out bus_rdy : ns(1);
          out bus_dataout : ns(32);
          in bus_wait : ns(1);
          
          // Slave ports - data going in to the slave does not need a handshake. 
          in datain : ns(32); // data from main unit to interface
          in datainrdy : ns(1); // slave signals that data and cmd is rdy
          out dataout : ns(32); // data to slave
          out cmdout : ns(32); // cmd to slave
          out dataoutrdy : ns(1); // signal to slave telling that the data on dataout is rdy
          in targetID : ns(4); // specifies the ID used in the MSB of CMD, which is intended for this slave
          out stallSlave : ns(1) // true if the slaveinterface is currently busy and cannot receive data from the slave
          ){
          
          reg bus_reqr : ns(1);
          reg bus_waitr : ns(1);
          reg datainrdyr : ns(1);
          reg targetIDr : ns(4);
          reg bus_cmdr : ns(32);
          
          always{
             bus_reqr = bus_req;
             bus_waitr = bus_wait;
             datainrdyr = datainrdy;
             targetIDr = targetID;
             bus_cmdr = bus_cmd;
          }
          
          sfg externalData{
             bus_ack = 1;
             dataout = bus_datain;
             cmdout = bus_cmd;
             dataoutrdy = bus_cmd[28:31] == targetID; // is the cmd and data intended for this slave?
             
             bus_rdy = 0;
             bus_dataout = 0;
             stallSlave = 1;
          }
          
          sfg ackExternalData{
             bus_ack = 0;
             dataout = 0;
             cmdout = 0;
             dataoutrdy = 0;
             bus_rdy = 0;
             bus_dataout = 0;
             stallSlave = 1;
          }
          
          sfg internalData{
             bus_dataout = datain;
             bus_rdy = 1;
             
             bus_ack = 0;
             dataout = 0;
             cmdout = 0;
             dataoutrdy = 0;
             stallSlave = 1;
          }
          
          sfg ackInternalData{
             bus_rdy = 0;
             bus_dataout = 0;
             
             bus_ack = 0;
             dataout = 0;
             cmdout = 0;
             dataoutrdy = 0;
             stallSlave = 1;
          }
          
          sfg nothing{
             bus_ack = 0;
             dataout = 0;
             cmdout = 0;
             dataoutrdy = 0;
             bus_rdy = 0;
             bus_dataout = 0;
             stallSlave = 0;
          }
          
}

fsm slavebusinterface_cntrl(slavebusinterface){
   initial s0;
   state s1,s2,s3;
   
   // condition explanation: Only ack if the cmd is intended for this slave
   @s0 if (bus_reqr == 1 & bus_cmdr[28:31] == targetIDr) then (externalData) -> s1;
       else (nothing) -> s0;
   @s1 if (bus_reqr == 0) then (ackExternalData) ->s2;
       else (externalData) -> s1;
   @s2 if (datainrdyr == 1) then (internalData) -> s3;
       else (nothing) -> s2;
   @s3 if (bus_waitr == 1) then (ackInternalData) -> s0;
       else (internalData) -> s3;
   
}

dp slavebusinterface2 : slavebusinterface
dp slavebusinterface3 : slavebusinterface

dp masterbusinterface(
          // Bus ports
          out bus_req : ns(1); // master request the bus
          out bus_cmd : ns(32); // cmd to target unit
          out bus_dataout : ns(32); // data to target unit
          in bus_ack : ns(1); // ack from bus
                      
          in bus_rdy : ns(1); // rdy from bus
          in bus_datain : ns(32); // data targeted for the master
          out bus_wait : ns(1); // wait to bus
                    
          // Master ports - data going in to the master does not need a handshake. 
          in datain : ns(32); // data from main unit to interface
          in cmd : ns(32); // command from main unit to interface
          in datainrdy : ns(1); // master signals that data and cmd is rdy
          out dataout : ns(32); // data to master
          out dataoutrdy : ns(1) // signal to master telling that the data on dataout is rdy
          ){
          
//          $trace(bus_req, "traces/M_bus_req.seq");
//          $trace(bus_cmd, "traces/M_bus_cmd.seq");
//          $trace(bus_dataout, "traces/M_bus_dataout.seq");
//          $trace(bus_ack, "traces/M_bus_ack.seq");
//          $trace(bus_rdy, "traces/M_bus_rdy.seq");
//          $trace(bus_datain, "traces/M_bus_datain.seq");
//          $trace(bus_wait, "traces/M_bus_wait.seq");
          
//          $trace(datain, "traces/M_datain_int.seq");
//          $trace(cmd, "traces/M_cmd_int.seq");
//          $trace(datainrdy, "traces/M_datainrdy_int.seq");
//          $trace(dataout, "traces/M_dataout_int.seq");
//          $trace(dataoutrdy, "traces/M_dataoutrdy_int.seq");
          
          reg bus_ackr : ns(1);
          reg bus_rdyr : ns(1);
          reg datainrdyr : ns(1);
          reg cmdr : ns(32);
          reg datainr : ns(32);
                      
          always{
             bus_ackr = bus_ack;
             bus_rdyr = bus_rdy;
             datainrdyr = datainrdy;
             cmdr = cmd;
             datainr = datain;
          }
          
          sfg internalData{
             bus_req = 1;
             bus_cmd = cmd;
             bus_dataout = datain;
             
             bus_wait = 0;
             dataout = 0;
             dataoutrdy = 0;
          }
          
          sfg ackInternalData{
             bus_req = 0;
             bus_cmd = cmdr;
             bus_dataout = datainr;
             
             bus_wait = 0;
             dataout = 0;
             dataoutrdy = 0;
          }
          
          sfg externalData{
             bus_wait = 1;
             dataout = bus_datain;
             dataoutrdy = 1;
             
             bus_req = 0;
             bus_cmd = 0;
             bus_dataout = 0;
          }
          
          sfg ackExternalData{
              bus_wait = 0;
              dataout = 0;
              dataoutrdy = 0;
              
              bus_req = 0;
              bus_cmd = 0;
              bus_dataout = 0;
          }
                    
          sfg nothing{
             bus_req = 0;
             bus_cmd = cmdr;
             bus_dataout = datainr;
             
             bus_wait = 0;
             dataout = 0;
             dataoutrdy = 0;
          }
}

fsm masterbusinterface_cntrl(masterbusinterface){
   initial s0;
   state s1,s2,s3,s4;
   
   @s0 if (datainrdyr == 1) then (internalData) -> s1;
       else (nothing) -> s0; // do nothing
   @s1 if (bus_ackr == 1) then (ackInternalData) -> s3; // go to s3 in order to make sure the bus will not initiate a new communication due to the fact that the CPU is stalled at this particular bus-using instruction
       else (internalData) -> s1; // do nothing
   @s3 if (bus_rdyr == 1) then (externalData) -> s2;
       else (nothing) -> s3;
   @s2 (ackExternalData) -> s4; 
   @s4 (nothing) -> s0; // Wait one more cycle to synchronize with PC
}

dp bus_toplevel(
           // Master interface
           in M_datain : ns(32);
           in M_cmd : ns(32);
           in M_datainrdy : ns(1);
           out M_dataout : ns(32);
           out M_dataoutrdy : ns(1);
           //out stall_cpu : ns(1);
           
           // Slave interface (datamemory)
           in DM_datain : ns(32);
           in DM_datainrdy : ns(1);
           out DM_dataout : ns(32);
           out DM_cmdout : ns(32);
           out DM_dataoutrdy : ns(1);
           out DM_stall : ns(1);
           
           // Slave interface (Sensor)
           in SS_datain : ns(32);
           in SS_datainrdy : ns(1);
           out SS_dataout : ns(32); // data port not used in sensor
           out SS_cmdout : ns(32);
           out SS_dataoutrdy : ns(1);
           out SS_stall : ns(1);
           
           // Slave interface (output)
           in OP_datain : ns(32);
           in OP_datainrdy : ns(1);
           out OP_dataout : ns(32); // data port not used in sensor
           out OP_cmdout : ns(32);
           out OP_dataoutrdy : ns(1);
           out OP_stall : ns(1)
           ){
           
           sig reqCPU,reqDataMem,reqSensor,reqOutput,ackCPU,ackDataMem,ackSensor,ackOutput,dataRdyCPU,dataRdyDataMem,dataRdySensor,dataRdyOutput,waitCPU,waitDataMem,waitSensor,waitOutput: ns(1);
           sig cmdCPU,cmdDataMem,cmdSensor,cmdOutput,dataoutCPU,datainDataMem,datainSensor,datainOutput,datainCPU,dataoutDataMem,dataoutSensor,dataoutOutput : ns(32);
           reg ID_DataMem : ns(4);
           reg ID_Sensor : ns(4);
           reg ID_Output : ns(4);
           
           $trace(DM_datain, "traces/DM_datain.seq");
           $trace(DM_datainrdy, "traces/DM_datainrdy.seq");
           $trace(DM_dataout, "traces/DM_dataout.seq");
           $trace(DM_cmdout, "traces/DM_cmdout.seq");
           $trace(DM_dataoutrdy, "traces/DM_dataoutrdy.seq");
           
           $trace(SS_datain, "traces/SS_datain.seq");
           $trace(SS_datainrdy, "traces/SS_datainrdy.seq");
           $trace(SS_dataout, "traces/SS_dataout.seq");
           $trace(SS_cmdout, "traces/SS_cmdout.seq");
           $trace(SS_dataoutrdy, "traces/SS_dataoutrdy.seq");
           
           $trace(OP_datain, "traces/OP_datain.seq");
           $trace(OP_datainrdy, "traces/OP_datainrdy.seq");
           $trace(OP_dataout, "traces/OP_dataout.seq");
           $trace(OP_cmdout, "traces/OP_cmdout.seq");
           $trace(OP_dataoutrdy, "traces/OP_dataoutrdy.seq");
           
           use bus(reqCPU,cmdCPU,dataoutCPU,ackCPU,dataRdyCPU,datainCPU,waitCPU, // master
                   reqDataMem,cmdDataMem,datainDataMem,ackDataMem,dataRdyDataMem,dataoutDataMem,waitDataMem, // Data memory (slave)
                   reqSensor,cmdSensor,datainSensor,ackSensor,dataRdySensor,dataoutSensor,waitSensor,
                   reqOutput,cmdOutput,datainOutput,ackOutput,dataRdyOutput,dataoutOutput,waitOutput,// Sensor (slave)
                   ID_Sensor,ID_DataMem,ID_Output); 
           
           use masterbusinterface(reqCPU,cmdCPU,dataoutCPU,ackCPU,dataRdyCPU,datainCPU,waitCPU, // Bus interface
                                  //CPUdatain,CPUcmd,CPUdatainrdy,CPUdataout,CPUdataoutrdy
                                  M_datain,M_cmd,M_datainrdy,M_dataout,M_dataoutrdy);

           use slavebusinterface(reqDataMem,cmdDataMem,datainDataMem,ackDataMem,dataRdyDataMem,dataoutDataMem,waitDataMem, // Bus interface (data memory)
                             //DMDatain,DMDatainrdy,DMDataout,DMCmdout,DMDataoutrdy,
                             DM_datain,DM_datainrdy,DM_dataout,DM_cmdout,DM_dataoutrdy,ID_DataMem,DM_stall);
                             
           use slavebusinterface2(reqSensor,cmdSensor,datainSensor,ackSensor,dataRdySensor,dataoutSensor,waitSensor, // Bus interface (Sensor)
                              //SensorDatain,SensorDatainrdy,NotUsed2,SensorCmdout,SensorDataoutrdy,
                              SS_datain,SS_datainrdy,SS_dataout,SS_cmdout,SS_dataoutrdy,ID_Sensor,SS_stall);
                              
           use slavebusinterface3(reqOutput,cmdOutput,datainOutput,ackOutput,dataRdyOutput,dataoutOutput,waitOutput, // Bus interface (data memory)
                              //OPDatain,OPDatainrdy,OPDataout,OPCmdout,OPDataoutrdy,
                              OP_datain,OP_datainrdy,OP_dataout,OP_cmdout,OP_dataoutrdy,ID_Output,OP_stall);
           
           always{
              ID_Sensor = 0x1;
              ID_DataMem = 0x2;
              ID_Output = 0x3;
           }
}

////////////////////////////
// Auxiliary units
////////////////////////////
ipblock datamem(in address : ns(12);
          in wr,rd   : ns(1);
          in idata   : ns(32);
          out odata : ns(32)){

  iptype "ram";
  ipparm "size=4096";
  ipparm "wl=32";
  ipparm "file=datamemory.txt"; // initializes content. File syntax needs to be "addr data "* (the last space can also be a linebreak. NB! Should the students do a manual loading of the RAM to simulate a "real" system or is it ok if they just use this parameter?
  //ipparm "file=sddb48hex2.ecg"; 
}

dp DataMem(
   in datain : ns(32);
   in cmd : ns(32);
   in datainrdy : ns(1);
   out dataout : ns(32);
   out dataoutrdy : ns(1);
   in stall : ns(1) // true if the slave is currently busy (with handshaking) and cannot receive new data.
   ){
   
   sig wr,rd : ns(1);
   sig address : ns(12);
   sig idata : ns(32);
   
   reg datainrdyr : ns(1);
   reg cmdr : ns(32);
   reg datainr : ns(32);
   reg dataoutr : ns(32);
   reg stallr : ns(1);
   
   use datamem(address,wr,rd,idata,dataout);
   
   $trace(address, "traces/DM_address.seq");
   $trace(wr, "traces/DM_wr.seq");
   $trace(rd, "traces/DM_rd.seq");
   $trace(idata, "traces/DM_idata.seq");
   
   always{
       stallr = stall;
       rd = cmdr[27] == 0;
       wr = cmdr[27] == 1;
       address = (cmdr[27] == 0) ? datainr : 0b00000 # cmdr[0:26]; // if Store Word instruction, use cmd[0:26] as address. If Load Word instruction use datain as address
       idata = (cmdr[27] == 0) ? 0 : datainr; // if store word use datain as data to be stored, if load word, just set idata to 0
       //$display($dec, "Cycle: ", $cycle);
   	   //$display($dec, "   address: ", address);
   }
   
   sfg captureData{
       dataoutrdy = 0; 
       datainrdyr = datainrdy;
       datainr = datain;
       cmdr = cmd;
   }
   
   sfg awaithandshake{
      dataoutrdy = 0;
   }
   
   sfg resetAndSignalRdy{
     dataoutrdy = 1;
     datainrdyr = 0;
   }
   
   sfg nothing{
       dataoutrdy = 0; 
   }
   
}

fsm DataMem_cntrl(DataMem){
   initial s0;
   state s1;
   
   @s0 if (datainrdyr == 1 & stallr == 0) then (resetAndSignalRdy) -> s0;
       else if (datainrdyr == 1 & stallr == 1) then (nothing) -> s1;
       else (captureData) -> s0;
   @s1 if (stallr == 0) then (resetAndSignalRdy) -> s0;
       else (awaithandshake) -> s1;
       
}

ipblock sensordata(in address : ns(5);
          in wr,rd   : ns(1);
          in idata   : ns(32);
          out odata : ns(32)){

  iptype "ram";
  ipparm "size=32";
  ipparm "wl=32";
  ipparm "file=sddb48hex.ecg"; // initializes content
  
}

dp Sensor(
   in datain : ns(32);
   in cmd : ns(32);
   in datainrdy : ns(1);
   out dataout : ns(32);
   out dataoutrdy : ns(1);
   in stall : ns(1) // true if the slave is currently busy (with handshaking) and cannot receive new data.
   ){
   
    sig address : ns(5);
    sig wr,rd : ns(1);
    sig idata,odata : ns(32);
    
    use sensordata(address,wr,rd,idata,odata);

   	reg c : ns(10);
   	reg reset : ns(1);
   	reg addr : ns(8);
   	reg LIMIT : ns(10); // How many cycles to wait before it "acquires" the next datapoint (simulated sampling rate)
   	reg stallr : ns(1);
   	reg datainrdyr : ns(1);
   	
   	$trace(odata, "traces/SS_data.seq");
   	$trace(c, "traces/SS_c.seq");
   	
   	always{
   		wr = 0;
   		idata = 0;
   		LIMIT = 9; // 0 means 1 cycles => Pick new point each cycle TODO: Set proper sampling rate
   		address = addr;
   		
   		dataout = odata;
   		//dataoutrdy = 0;
   		stallr = stall;
   		//$display($dec, "Cycle: ", $cycle);
   		//$display($dec, "   data: ", odata);
   		//$display($dec, "   address: ", address);
   	}
   	
   sfg captureData{  
       dataoutrdy = 0; 
       datainrdyr = datainrdy;
       // Removed all datain and cmd capturing registers, due to the fact that the sensor only has one function, which is to send data once requested
   }
   
   sfg awaithandshake{
      dataoutrdy = 0;
   }
   
   sfg resetAndSignalRdy{
     dataoutrdy = 1;
     datainrdyr = 0;
   }
   
   sfg nothing{
       dataoutrdy = 0; 
   }

	sfg count{
    	c = c + 1;
    	rd = 0;
	}
   	
   	sfg addressUpdate{
   	   addr = addr + 1;
   	   rd = 1;
   	   c = 0;
   	}

}

fsm sensor_cntrl(Sensor){
   initial s0;
   state s1;
       
   @s0 if (datainrdyr == 1 & stallr == 0 & c == LIMIT) then (addressUpdate,resetAndSignalRdy) -> s0;
       else if (datainrdyr == 1 & stallr == 0 &  c != LIMIT) then (count,resetAndSignalRdy) -> s0;
       else if (datainrdyr == 1 & stallr == 1 & c == LIMIT) then (addressUpdate,nothing) -> s1;
       else if (datainrdyr == 1 & stallr == 1 & c != LIMIT) then (count,nothing) -> s1;
       else if (datainrdyr == 0 & c == LIMIT) then (addressUpdate,captureData) -> s0;
       else (count,captureData) -> s0;
   @s1 if (stallr == 0 & c == LIMIT) then (addressUpdate,resetAndSignalRdy) -> s0;
       else if (stallr == 0 & c != LIMIT) then (count,resetAndSignalRdy) -> s0;
       else if (stallr == 1 & c == LIMIT) then (addressUpdate,awaithandshake) -> s1;
       else (count,awaithandshake) -> s1;
}

dp Output(
   in datain : ns(32);
   in cmd : ns(32);
   in datainrdy : ns(1);
   out dataout : ns(32);
   out dataoutrdy : ns(1);
   in stall : ns(1); // true if the slave is currently busy (with handshaking) and cannot receive new data.
   out FilterOut : tc(32)
   ){
   
   reg datainrdyr : ns(1);
   reg cmdr : ns(32);
   reg datainr : ns(32);
   reg dataoutr : ns(32);
   reg stallr : ns(1);
   reg count : ns(10);
   reg FilterOut_r : tc(32);
   
   $trace(FilterOut, "traces/FilterOut.seq");
   $trace(count, "traces/out_count.seq");
   
   always{
       stallr = stall;
       dataout = 0x123;
   }
   
   sfg captureData{
       dataoutrdy = 0; 
       datainrdyr = datainrdy;
       datainr = datain;
       cmdr = cmd;
       
       FilterOut = FilterOut_r;
   }
   
   sfg awaithandshake{
      dataoutrdy = 0;
      
      FilterOut = FilterOut_r;
   }
   
   sfg resetAndSignalRdy{
     dataoutrdy = 1;
     datainrdyr = 0;
   }
   
   sfg nothing{
       dataoutrdy = 0;
       
       FilterOut = FilterOut_r;
   }
   
   sfg setFilterOut{
      FilterOut = datainr;
      FilterOut_r = datainr;
      count = count + 1;
   }
   
}

fsm Output_cntrl(Output){
   initial s0;
   state s1;
   
   @s0 if (datainrdyr == 1 & stallr == 0) then (resetAndSignalRdy,setFilterOut) -> s0;
       else if (datainrdyr == 1 & stallr == 1) then (nothing) -> s1;
       else (captureData) -> s0;
   @s1 if (stallr == 0) then (resetAndSignalRdy,setFilterOut) -> s0;
       else (awaithandshake) -> s1;
       
}

////////////////////////////
// System connections
////////////////////////////
system CPUsystem{

   CPU(CPUdatain,CPUcmd,CPUdatainrdy,CPUdataout,CPUdataoutrdy);
   
   bus_toplevel(CPUdatain,CPUcmd,CPUdatainrdy,CPUdataout,CPUdataoutrdy, // master unit
           DMDatain,DMDatainrdy,DMDataout,DMCmdout,DMDataoutrdy,DMStall, // slave (data memory)
           SensorDatain,SensorDatainrdy,SensorDataout,SensorCmdout,SensorDataoutrdy,SensorStall,  // slave (sensor)
           OPDatain,OPDatainrdy,OPDataout,OPCmdout,OPDataoutrdy,OPStall); // slave (Output)
           
   DataMem(DMDataout,DMCmdout,DMDataoutrdy,DMDatain,DMDatainrdy,DMStall); // Data Memory Unit
   Sensor(SensorDataout,SensorCmdout,SensorDataoutrdy,SensorDatain,SensorDatainrdy,SensorStall);
   Output(OPDataout,OPCmdout,OPDataoutrdy,OPDatain,OPDatainrdy,OPStall,FilterOut);
}