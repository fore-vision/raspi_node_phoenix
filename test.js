const {
    rfid
} = require('bindings')('phoenixreader')


const rfid1 = new rfid();
let ret = rfid1.Open('/dev/ttyUSB0');
console.log("open = " + ret);
ret = rfid1.GetMaxPower();
console.log("GetMaxpower = " + ret);
rfid1.Close();