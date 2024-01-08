
const express = require('express');
const app = express();
const port = 3001;

app.get('/', (req, res) => {
  res.send('Hello, World!');
});

app.use('/static', express.static("../public/router_1"));

app.listen(port, () => {
  console.log(`Server is listening at http://localhost:${port}`);
});
