import { SerialPort, ReadlineParser } from "serialport";

const port = new SerialPort({
  path: "/dev/ttyACM0",
  baudRate: 115200,
  // baudRate: 921600,
});

port.on("open", () => {
  console.log("serial port was open");
  loop();
});

const parser = port.pipe(new ReadlineParser());
parser.on("data", (data) => {
  console.log(`data: '${data}'`);
  if (data[0] === "$") {
    // convert char to number
    console.log("pressed: ", data[1].charCodeAt(0));
  } else if (data[0] === "!") {
    console.log("released: ", data[1].charCodeAt(0));
  } else {
    console.log("unknown: ", data);
  }
});

const sleep = (ms: number) => new Promise((resolve) => setTimeout(resolve, ms));

const buttonCount = 12 * 4;
const colorCount = 16 * 10; // 16 colors of 10 brightness
// 255 is reserved for no color

async function loop() {
  const state: Uint8Array = new Uint8Array(3);

  state[0] = 0x23; // equivalent to '#' command to set color
  for (let i = 7; i < buttonCount; i++) {
    state[1] = i;
    for (let j = 0; j < colorCount; j++) {
      state[2] = j;
      port.write(state);
      // console.log({state});
      await sleep(50);
    }
    state[2] = 255; // button color off
    port.write(state);
    // console.log({state});
    await sleep(50);
  }
}
