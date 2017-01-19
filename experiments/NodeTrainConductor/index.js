const SerialPort = require('serialport');
const WebSocket = require('ws');

const config = {
    train: {
        port: 'COM11',
        baudrate: 115200,
        delimiter: '\n'
    },
    chargeFindSpeed:80,
    chargeReverseSpeed: 60
};

const serial = new SerialPort(config.train.port, {
    baudrate: config.train.baudrate,
    parser: SerialPort.parsers.readline(config.train.delimiter)
});

let isStoppingToCharge = false;
let isOnTagReader = false;
let lastTrainSpeed = 0;
let onTagTimeout = null;

serial.name = 'train';
serial.sendMessage = (message, callback) => {
    sendMessage(serial, message, callback);
}

const ws = new WebSocket('ws://localhost:3000');

ws.sendMessage = (message) => {
    console.log(`ws < ${message}`);

    ws.send(message);
}

ws.on('open', () => {
    console.log('websocket opened');
});

ws.on('message', (message, flags) => {
    console.log('ws > ' + message);

    if (message === 'charge') {
        stopToCharge();
    } else if (message === 'test-node-server') {
        console.log('testing server..');

        setMotorSpeed(-100);
    }
});

serial.on('open', () => {
    console.log('port opened');

    serial.sendMessage('1:port:2:PN532:enable');
});

serial.on('data', (message) => {
    console.log(`${serial.name} ${message}`);

    if (message.indexOf('enter:TRAIN') !== -1) {
        console.log('# train entered tag');

        serial.sendMessage('1:port:3:TLC5940:value:0:1');

        isOnTagReader = true;

        if (isStoppingToCharge) {
            console.log('# stopping train');

            //brakeMotor();
            stopMotor();

            onTagTimeout = setTimeout(() => {
                console.log('# now charging');

                isStoppingToCharge = false;
                onTagTimeout = null;
            }, 2000);
        }
    } else if (message.indexOf('exit:TRAIN') !== -1) {
        console.log('# train exited tag');

        isOnTagReader = false;

        if (isStoppingToCharge) {
            console.log('# still stopping to charge');

            if (lastTrainSpeed > 0) {
                console.log('# backing up');

                setMotorSpeed(-config.chargeReverseSpeed);

                clearTimeout(onTagTimeout);
            } else if (lastTrainSpeed < 0) {
                console.log('# going forward');

                setMotorSpeed(config.chargeReverseSpeed);
                clearTimeout(onTagTimeout);
            }
        }

        serial.sendMessage('1:port:3:TLC5940:value:0:0');
    }
});

serial.on('error', (err) => {
    console.error(err.message);
});

function sendMessage(serial, message, callback) {
    if (typeof callback !== 'function') {
        callback = (error) => {
            if (error) {
                console.error(`${serial.name} @ sending message ${message} failed: ${error}`);
            } else {
                //console.log(`${serial.name} > ${message}`);
            }
        }
    }

    serial.write(message + '\n', callback);
}

function stopToCharge() {
    isStoppingToCharge = true;

    setMotorSpeed(config.chargeFindSpeed);
}

function setMotorSpeed(speed = 100) {
    ws.sendMessage(`motor:${speed}`);

    if (speed !== 0) {
        lastTrainSpeed = speed;
    }
}

function stopMotor() {
    setMotorSpeed(0);
}

function brakeMotor() {
    ws.sendMessage('brake');
}
