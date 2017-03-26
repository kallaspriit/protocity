import path from 'path';
import { spawn } from 'child_process';
import opn from 'opn';
import handleDeath from 'death';

const config = {
    app: {
        baseUrl: 'http://localhost:3000'
    },
    env: {
        resolution: 1080,
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
    },
    (errorBuffer) => {
        const message = errorBuffer.toString();

        // console.error(message);
    },
    (code, signal) => {
        console.log(`cumulocity agent closed (${code} - ${signal})`);
    }
);

function handleGatewayRunning() {
    console.log(`gateway is now running, starting dashboard server`);

    // start dashboard server
    runCommand(
        'npm start', [], {
            cwd: config.paths.dashboard
        },
        (outputBuffer) => {
            const message = outputBuffer.toString();

            if (message.indexOf('app is running') !== -1) {
                handleDashboardRunning();
            }

            // console.info(message);
        },
        (errorBuffer) => {
            const message = errorBuffer.toString();

            // console.error(message);
        },
        (code, signal) => {
            console.log(`dashboard server closed (${code} - ${signal})`);
        }
    );
}

function handleDashboardRunning() {
    console.log(`dashboard is now running, opening browser windows`);

    config.screens.forEach(screen => openScreen(screen));
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
