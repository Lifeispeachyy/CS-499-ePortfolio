const mongoose = require('./db');
const Trip = require('./travlr');
const fs = require('fs');

const trips = JSON.parse(fs.readFileSync('./data/trips.json', 'utf8'));

async function seedDB() {
  try {
    await Trip.deleteMany({});
    console.log('Old data removed');

    await Trip.insertMany(trips);
    console.log('Database seeded');

    mongoose.connection.close();
  } catch (err) {
    console.log(err);
  }
}

seedDB();