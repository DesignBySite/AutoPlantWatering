require('dotenv').config();

module.exports = {
  mongoUser: process.env.MONGO_USER,
  mongoPass: process.env.MONGO_PASSWORD,
  mongoUri: `mongodb+srv://${process.env.MONGO_USER}:${process.env.MONGO_PASSWORD}@autowatering.irtdy.mongodb.net/?retryWrites=true&w=majority&appName=AutoWatering`
}