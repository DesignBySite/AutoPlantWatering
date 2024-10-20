const http = require('http');

const getCurrentTime = () => {
  let now = new Date();
  return `${now.getHours()}:${now.getMinutes()}:${now.getSeconds()}`
}

const updateDataAndNotifyClients = (data) => {
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

module.exports = {
  getCurrentTime,
  updateDataAndNotifyClients,
  sendSensorData
}