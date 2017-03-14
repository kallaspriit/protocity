const SerialPort = require('serialport');
const net = require('net');

const config = {
    serial: {
        port: 'COM24',
        baudrate: 115200,
        delimiter: '\n',
        isUsed: false,
    },
    socket: {
        host: '10.220.20.204',
        port: 8080,
        isLoggingEnabled: false
    }
};

// setup serial
const serial = new SerialPort(config.serial.port, {
    baudrate: config.serial.baudrate,
    parser: SerialPort.parsers.readline(config.serial.delimiter),
    autoOpen: false
});

let pingSendTime = 0;
let pingCount = 0;

serial.sendMessage = (message, callback) => {
    if (typeof callback !== 'function') {
        callback = (error) => {
            if (error) {
                console.error(`@ sending message '${message}' to ${config.serial.port} failed: ${error}`);

                return;
            }

            console.log(`${config.serial.port} < ${message}`);
        }
    }

    serial.write(message + '\n', callback);
}

serial.on('open', () => {
    console.log(`# connected to serial port ${config.serial.port}`);

    connectToSocket();
    // serial.sendMessage('1:port:1:TLC5940:test');
});

serial.on('data', (message) => {
    console.log(`${config.serial.port} > ${message}`);

    /*
    if (message.indexOf('waiting for new connection') !== -1) {
        connectToSocket();
    }
    */
});

serial.on('error', (error) => {
    console.error(`@ ${config.serial.port} error: ${error.message}`);
});

serial.on('close', () => {
    console.log(`# closed serial port ${config.serial.port}`);
});

// setup socket
const socket = new net.Socket();

socket.setEncoding('utf8');

socket.isOpen = false;

socket.sendMessage = (message, callback) => {
    if (!socket.isOpen) {
        console.error(`@ sending message '${message}' to ${config.socket.host}:${config.socket.port} failed: socket is not open`);

        return;
    }

    if (typeof callback !== 'function') {
        callback = (error) => {
            if (error) {
                console.error(`@ sending message '${message}' to ${config.socket.host}:${config.socket.port} failed: ${error}`);

                return;
            }

            if (config.socket.isLoggingEnabled) {
                console.log(`${config.socket.host}:${config.socket.port} < ${message}`);
            }
        }
    }

    socket.write(message + '\n', callback);
}

socket.on('connect', () => {
    console.log(`# connected to socket at ${config.socket.host}:${config.socket.port}`);

    socket.isOpen = true;

    sendPing();

    // setInterval(() => {
    //     sendPing();
    // }, 50);
});

socket.on('data', (rawMessage) => {
    if (typeof rawMessage !== 'string') {
        console.log(`${config.socket.host}:${config.socket.port} > got a ${typeof rawMessage} (${rawMessage})`);

        return;
    }

    const message = rawMessage.replace(/\n/g, '');

    if (config.socket.isLoggingEnabled) {
        console.log(`${config.socket.host}:${config.socket.port} > ${message}`);
    }

    if (message == "1:OK:pong") {
        if (pingCount % 100 === 0) {
            const pingTimeTaken = now() - pingSendTime;

            console.log(`# ping #${pingCount} took ${pingTimeTaken} ms, requesting memory usage`);

            socket.sendMessage("2:memory");
        }

        sendPing();
    } else if (message.substring(0, 1) === '2') { // memory response
        console.log(`${config.socket.host}:${config.socket.port} > ${message}`);
    }
});

socket.on('error', (err) => {
	console.error(`@ ${config.socket.host}:${config.socket.port} error: ${err.message}`);
});

socket.on('close', function() {
	console.log(`# closed socket at ${config.socket.host}:${config.socket.port}`);

    socket.isOpen = false;
});

// startup logic
if (config.serial.isUsed) {
    console.log(`# opening serial port ${config.serial.port}`);

    serial.open();
} else {
    connectToSocket();
}

// helper functions
function connectToSocket() {
    console.log(`# connecting to socket at ${config.socket.host}:${config.socket.port}...`);

    socket.connect({
        host: config.socket.host,
        port: config.socket.port,
    });
}

function sendPing() {
    pingSendTime = now();
    pingCount++;

    socket.sendMessage('1:ping');
}

function now() {
    return Date.now();
}
