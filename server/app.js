const express = require('express');
const app = express();
const { getCurrentTime, updateDataAndNotifyClients } = require('./sensorCommunication/sensors');
app.use(express.json()); // Middleware to parse JSON bodies

app.post('/data', (req, res) => {
  const newData = req.body; // Assume data comes in the request body
  let currentTime = getCurrentTime();
  // Update your data here
  console.log("Current Time: ", currentTime, "Data: ", newData);
  updateDataAndNotifyClients(newData);
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