const express = require('express');
const app = express();
const { getCurrentTime } = require('./sensorCommunication/sensors');
const { pushToDataBuffer } = require('./mongoDB/mongo')

app.use(express.json()); // Middleware to parse JSON bodies

app.post('/data', (req, res) => {
  const newData = req.body;
  const currentTime = getCurrentTime();
  newData.data.date_time = new Date();
  pushToDataBuffer(newData.data);

  console.log("Current Time:", currentTime, "Data:", newData.data);
  res.status(200).send('Data received successfully');
});

app.get('/events', (req, res) => {
  res.setHeader('Content-Type', 'text/event-stream');
  res.setHeader('Cache-Control', 'no-cache');
  res.setHeader('Connection', 'keep-alive');

  // Add this client to the clients array
  clients.push(res);

  req.on('close', () => {
      console.log('Client disconnected from SSE');
      clients = clients.filter(client => client !== res);
      res.end();
  });
});

module.exports = app