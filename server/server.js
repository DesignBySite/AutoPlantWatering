const app = require('./app');
const { run } = require('./mongoDB/mongo');
require('dotenv').config();

run(); 

const PORT = process.env.PORT || 3051;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
