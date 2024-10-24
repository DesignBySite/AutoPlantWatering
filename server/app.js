const express = require('express');
const app = express();
const cors = require('cors');
const { getCurrentTime } = require('./sensorCommunication/sensors');
const { pushToDataBuffer } = require('./mongoDB/mongo')

app.use(cors());
app.use(express.json()); // Middleware to parse JSON bodies

let clients = [];

app.post('/data', (req, res) => {
  const newData = req.body;
  const currentTime = getCurrentTime();
  newData.data.date_time = new Date();
  pushToDataBuffer(newData.data, clients);

  console.log("Current Time:", currentTime, "Data:", newData.data);
  res.status(200).send('Data received successfully');
});

app.get('/events', (req, res) => {
  res.setHeader('Content-Type', 'text/event-stream');
  res.setHeader('Cache-Control', 'no-cache');
  res.setHeader('Connection', 'keep-alive');

  // Add this client to the clients array
  clients.push(res);
  console.log('Client connected to SSE');

  req.on('close', () => {
      console.log('Client disconnected from SSE');
      clients = clients.filter(client => client !== res);
      res.end();
  });
});

module.exports = app