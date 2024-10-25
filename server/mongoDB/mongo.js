const { MongoClient } = require('mongodb');
const { mongoUri } = require('../keys');

const sensorDataBuffer = [];
const BUFFER_SIZE = 50;
const INSERT_INTERVAL = 5000; // 5 seconds
let isInserting = false;

/**
 * Connects to the database
 * @returns client db
 */
let client;
let clientPromise;

const connectToDatabase = async () => {
  try {
    if (!clientPromise) {
      client = new MongoClient(mongoUri);

      clientPromise = client.connect().then(() => {
        console.log('Connected to MongoDB');
        return client.db('WateringDB');
      }).catch((error) => {
        console.error('Error connecting to MongoDB:', error);
        client = null;
        clientPromise = null;
        throw error;
      });
    }
    return clientPromise;
  } catch (error) {
    console.error('Failed to connect to MongoDB:', error);
    throw error;
  }
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

const getSensors = async (options = {}) => {
  try {
    const database = await connectToDatabase();
    const collection = database.collection('sensorReadings'); // Corrected method

    let query = {};

    let startDate, endDate;

    if (options.period) {
      // Calculate startDate and endDate based on the period
      endDate = new Date(); // Current date and time
      switch (options.period) {
        case 'day':
          startDate = new Date(endDate.getTime() - 24 * 60 * 60 * 1000);
          break;
        case 'week':
          startDate = new Date(endDate.getTime() - 7 * 24 * 60 * 60 * 1000);
          break;
        case 'month':
          startDate = new Date(endDate.getTime() - 30 * 24 * 60 * 60 * 1000);
          break;
        default:
          startDate = new Date(0); // Default to epoch start if period is unrecognized
          break;
      }
    } else if (options.startDate || options.endDate) {
      // Use provided startDate and/or endDate
      startDate = options.startDate ? new Date(options.startDate) : new Date(0);
      endDate = options.endDate ? new Date(options.endDate) : new Date();
    }

    if (startDate && endDate) {
      query.date_time = { $gte: startDate, $lte: endDate };
    }

    const sensorData = await collection.find(query).toArray();
    return sensorData;
  } catch (error) {
    console.error('Error retrieving sensor data:', error);
    throw error; // Re-throw the error to handle it in the calling function
  }
};

module.exports = {
  connectToDatabase,
  pushToDataBuffer,
  getSensors
}