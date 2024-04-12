const express = require('express');
const app = express();

app.use(express.json()); // Middleware to parse JSON bodies

app.post('/data', (req, res) => {
  console.log('Data received:', req.body);
  res.status(200).send('Data received successfully');
});

const PORT = 3000;
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
