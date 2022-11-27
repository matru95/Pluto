const express = require('express');
var helmet = require('helmet');
const fs = require('fs');
var cors = require('cors');
const app = express();
require('dotenv').config();

let port = process.env.PORT || 5000;
let origin = 'https://robotics-dfea2.web.app';


// Middlewares
app.use(helmet());
app.use(cors({origin: origin}));
app.use(express.json());


/*
* Opens the local JSON file and reads the current command
*/ 
function get_command(){
    let rawdata = fs.readFileSync('./command.json');
    let data = JSON.parse(rawdata);
    return data;
}


/*
* Opens the local JSON file and updates the current command
*/ 
function update_command(status, command){
    let data = JSON.stringify({status: status, command: command});
    fs.writeFileSync('./command.json', data);
}


/*
* Returns the current command to the client.
* 
* Output: 
* @returns {command: <VALUE>}: Object containing the current command.
*/ 
app.get('/', (req, res) =>{
    let command = get_command();
    return res.status(200).send(command);
})


/*
* Sets the current command from the client.
* 
* Input:
* @command: new command
*
* Output: 
* @returns {200}: if the process ends correctly.
*/ 
app.post('/', (req, res) =>{
    let command = (req.body.command).toString();

    update_command("busy", command);

    // Auto Reset of the command after a fixed time in order to prevent robot's dead lock.
    setTimeout(function(){
        let command = get_command();
        if(command.status === "busy"){
            update_command("free", null);
        }
    },30000);

    return res.sendStatus(200);
})


/*
* Returns the current command to the Robot.
* 
* Input:
* @authorization: authorization code.
*
* Output: 
* @returns {command: <VALUE>}: if the process ends correctly returns the current command with a 200 status code.
* @returns {401}: if the authorization code is not correct.
*/ 
app.post('/command', (req, res) =>{
    let authorization = req.body.authorization;

    if(authorization !== process.env.AUTHORIZATION)
        return res.sendStatus(401);

    let command = get_command();
    return res.status(200).send({command: command.command});
})


/*
* Updates the current command.
* 
* Input:
* @authorization: authorization code.
* @status: new status (free/busy).
* @command: new command.
*
* Output: 
* @returns {200}: if the process ends correctly.
* @returns {401}: if the authorization code is not correct.
*/ 
app.post('/update', (req, res) =>{
    let authorization = req.body.authorization;
    let status = req.body.status;
    let command = req.body.command;

    if(authorization !== process.env.AUTHORIZATION)
        return res.sendStatus(401);

    update_command(status, command);
    return res.sendStatus(200);
})


/*
* Makes the web server listening of the specified port.
*/ 
app.listen(port, () =>{ console.log(`Application listening on port https://roboticsanddesign.herokuapp.com:${port}`)});