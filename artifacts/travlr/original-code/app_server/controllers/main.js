// var fs = require('fs');
// var trips = JSON.parse(fs.readFileSync('./data/trips.json', 'utf8'));

const tripsEndpoint = 'http://localhost:3000/api/trips';
const options = {
  method: 'GET',
  headers: {
    'Accept': 'application/json'
  }
};

const index = (req, res) => {
  res.render('index', { title: 'Travlr Getaways' });
};

const travel = async (req, res) => {
  fetch(tripsEndpoint, options)
    .then(res => res.json())
    .then(json => {
      if (!Array.isArray(json)) {
        return res
          .status(500)
          .send('API lookup error: response was not an array');
      } else if (!json.length) {
        return res
          .status(404)
          .send('API lookup error: no trips exist');
      } else {
        res.render('travel', {
          title: 'Travlr Getaways',
          trips: json
        });
      }
    })
    .catch(err => res.status(500).send(err.message));
};

module.exports = {
  index,
  travel
};