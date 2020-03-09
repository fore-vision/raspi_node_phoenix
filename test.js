const {
    rfid
} = require('bindings')('phoenixreader')


const rfid1 = new rfid();
let ret = rfid1.Open('/dev/ttyUSB0');
console.log("open = " + ret);
ret = rfid1.GetMaxPower();
console.log("GetMaxpower = " + ret);
rfid1.SetAntennaState(0, true, 100);
const obj = rfid1.GetAntennaState(0);
console.log(obj);

let process_end = () => {
    console.log("call stop");
    rfid1.Stop();
    console.log("call close");
    rfid1.Close();
    console.log("end");

};
//process.on('SIGINT', process_end);
//process.on('SIGTERM', process_end);
rfid1.Inventory((data) => {
    console.log(data)
});

setTimeout(process_end, 3 * 1000);