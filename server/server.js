const express = require('express');
const app = express();
const http = require('http');

let clients = [];
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

const getCurrentTime = () => {
  let now = new Date();
  return `${now.getHours()}:${now.getMinutes()}:${now.getSeconds()}`
}
const updateDataAndNotifyClients = (data) => {

  // Notify all clients
  clients.forEach(client => {
      client.write(`data: ${JSON.stringify(data)}\n\n`);
  });
}

const sendSensorData = (ipAddress, sensorNumber, safetyFlag, portNum) => {
  console.log("Sent Sensor Data: ", ipAddress, sensorNumber, safetyFlag, portNum);
  const data = JSON.stringify({
    sensorNumber: sensorNumber,
    safetyFlag: safetyFlag
  });

  const options = {
    hostname: ipAddress,
    port: portNum,
    path: '/updateFlag',
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Content-Length': Buffer.byteLength(data)
    }
  };

  const req = http.request(options, (res) => {
    console.log(`Status Code ${res.statusCode}`);
    
    res.on('data', (data) => {
      process.stdout.write(data);
    });
  });

  req.on('error', (err) => {
    console.error(`Error: ${err.message}`);
  });

  req.write(data);
  req.end();
}

// sendSensorData('10.0.0.235', 0, false, 80);
// sendSensorData('10.0.0.235', 1, false, 80);
// sendSensorData('10.0.0.235', 2, false, 80);
// sendSensorData('10.0.0.235', 3, false, 80);

const PORT = 3050;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
