import path from 'path';
import { spawn } from 'child_process';
import opn from 'opn';
import handleDeath from 'death';

const config = {
    app: {
        baseUrl: 'http://localhost'
    },
    env: {
        resolution: 1920,
    },
    paths: {
        dashboard: path.resolve(__dirname, '..', 'dashboard'),
        agent: path.resolve(__dirname, '..', 'cumulocity-agent'),
    },
    screens: [{
        display: 1,
        path: '/weather'
    }, {
        display: 2,
        path: '/train'
    }, {
        display: 3,
        path: '/truck'
    }, {
        display: 4,
        path: '/parking'
    }]
};

console.log('');
console.log("-- Lego City --")
console.log("starting lego city Cumulocity agent, then the dashboard server and finally open the browser windows");
console.log("the whole process will take several minutes, please be patient");
console.log("this script is located in protocity/scripts/run.js and triggered by protocity/run.bat");
console.log("starting Cumulocity agent (protocity/cumulocity-agent > debug)");
console.log("Cumulocity agent logs are available in protocity/cumulocity-agent/debug.log.txt, the logging can be configured from protocity/cumulocity-agent/cfg/logback-debug.xml")

// start cumulocity agent
runCommand(
    'debug', [], {
        cwd: config.paths.agent
    },
    (outputBuffer) => {
        const message = outputBuffer.toString();

        if (message.indexOf('gateway started') !== -1) {
            handleGatewayRunning();
        }

        // console.info(message);
        process.stdout.write(".");
    },
    (errorBuffer) => {
        const message = errorBuffer.toString();

        // console.error(message);
        process.stdout.write("#");
    },
    (code, signal) => {
        console.log(`cumulocity agent closed (${code} - ${signal})`);
    }
);

function handleGatewayRunning() {
    console.log('');
    console.log(`gateway is now running, starting dashboard server (protocity/dashboard > npm run build && npm run server)`);

    // start dashboard server
    runCommand(
        'npm run build && npm run server', [], {
            cwd: config.paths.dashboard
        },
        (outputBuffer) => {
            const message = outputBuffer.toString();

            if (message.indexOf('Serving!') !== -1) {
                handleDashboardRunning();
            }

            // console.info(message);
            process.stdout.write(".");
        },
        (errorBuffer) => {
            const message = errorBuffer.toString();

            // console.error(message);
            process.stdout.write("#");
        },
        (code, signal) => {
            console.log(`dashboard server closed (${code} - ${signal})`);
        }
    );
}

function handleDashboardRunning() {
    console.log('');
    console.log(`dashboard is now running, opening browser windows`);

    config.screens.forEach(screen => openScreen(screen));

    console.log("done!")
}


function runCommand(name, args, options, outputCallback, errorCallback, closeCallback) {
    const usedOptions = {
        encoding: 'utf8',
        ...options
    };

    // console.log('running command', name, args, usedOptions);

    const childProcess = spawn('cmd', ['/c', name, ...args], usedOptions);

    childProcess.stdout.on('data', outputCallback);
    childProcess.stderr.on('data', errorCallback);
    childProcess.on('close', closeCallback);
}

function openScreen({ display, path }) {
    const url = config.app.baseUrl + path;
    const options = {
        app: [
            'chrome',
            '--start-fullscreen',
            '--new-window',
            '--window-position=' + ((display - 1) * config.env.resolution) + ',0',
            '--user-data-dir=c:/chrome/screen' + display
        ]
    };

    opn(url, options);
}

handleDeath((signal, error) => {
    if (error) {
        console.log(`got error "${error}"shutting down (${signal})`);
    } else {
        console.log(`shutting down (${signal})`);
    }
});
