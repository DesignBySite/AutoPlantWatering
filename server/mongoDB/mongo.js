const { MongoClient, ServerApiVersion } = require('mongodb');
const { mongoUri } = require('../keys');

const sensorDataBuffer = [];
const BUFFER_SIZE = 50;
const INSERT_INTERVAL = 5000; // 5 seconds
let isInserting = false;

const client = new MongoClient(mongoUri, {
  serverApi: {
    version: ServerApiVersion.v1,
    strict: true,
    deprecationErrors: true,
  }
});

let isConnected = false;

/**
 * Connects to the database
 * @returns client db
 */
const connectToDatabase = async () => {
  if (!isConnected) {
    await client.connect();
    isConnected = true;
  }
  return client.db('WateringDB');
};

/**
 * Collects the sensor data object and sends it to array if good message
 * @param {Object} data the sensor data object
 * @returns 
 */
const pushToDataBuffer = (data, clients) => {
  const badMessage = '30 Minutes has elapsed';
  const outsideMessage = 'Outside while loop, no longer watering';
  const thresholdMessage = 'moisture above 10%, not watering';

  if (data.message === badMessage) {
    return;
  }

  if (sensorDataBuffer.length >= BUFFER_SIZE && !isInserting) {
    runDataSend();
  }

  if (data.message === outsideMessage || data.message === thresholdMessage) {
    sensorDataBuffer.push(data);
    console.log('data buffer');
    clients.forEach(client => {
      client.write('event: sensorUpdate\n');
      client.write(`data: ${JSON.stringify(data)}\n\n`);
    });
  }
  
};

/**
 * Runs batch sending to insert into db
 */
const runDataSend = () => {
  isInserting = true;
  const dataToInsert = sensorDataBuffer.splice(0, sensorDataBuffer.length);
  insertData(dataToInsert).finally(() => {
    isInserting = false;
  });
}

setInterval(() => {
  if (sensorDataBuffer.length > 0 && !isInserting) {
    runDataSend();
  }
}, INSERT_INTERVAL)

/**
 * Takes in array and inserts many into db
 * @param {Array} dataArray sensorDataBuffer array
 */
const insertData = async (dataArray) => {
  try {
    const database = await connectToDatabase();
    const collection = database.collection('sensorReadings');
    const result = await collection.insertMany(dataArray);
    console.log(`${result.insertedCount} documents were inserted.`);
  } catch (error) {
    console.error('Error inserting document:', error);
  }
};

module.exports = {
  connectToDatabase,
  pushToDataBuffer
}