const express = require('express');
const app = express();
const http = require('http');

app.use(express.json()); // Middleware to parse JSON bodies

app.post('/data', (req, res) => {
  console.log('Data received:', req.body);
  res.status(200).send('Data received successfully');
});

const sendSensorData = (ipAddress, sensorNumber, safetyFlag, portNum) => {
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

sendSensorData('10.0.0.235', 0, false, 80);

const PORT = 3050;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
