const mongoose = require('mongoose');
const Trip = mongoose.model('trips');

// GET all trips
const tripsList = async (req, res) => {
  try {
    const trips = await Trip.find().exec();
    res.status(200).json(trips);
  } catch (err) {
    res.status(500).json(err);
  }
};

// GET single trip by code
const tripsFindCode = async (req, res) => {
  try {
    const trip = await Trip.findOne({ code: req.params.tripCode }).exec();

    if (!trip) {
      return res.status(404).json({ message: 'Trip not found' });
    }

    res.status(200).json(trip);
  } catch (err) {
    res.status(500).json(err);
  }
};

// POST add new trip
const tripsAddTrip = async (req, res) => {
  try {
    const trip = await Trip.create({
      code: req.body.code,
      name: req.body.name,
      length: req.body.length,
      start: req.body.start,
      resort: req.body.resort,
      perPerson: req.body.perPerson,
      image: req.body.image,
      description: req.body.description
    });

    res.status(201).json(trip);
  } catch (err) {
    res.status(400).json(err);
  }
};

// PUT update trip by code
const tripsUpdateTrip = async (req, res) => {
  try {
    const trip = await Trip.findOneAndUpdate(
      { code: req.params.tripCode },
      {
        code: req.body.code,
        name: req.body.name,
        length: req.body.length,
        start: req.body.start,
        resort: req.body.resort,
        perPerson: req.body.perPerson,
        image: req.body.image,
        description: req.body.description
      },
      { new: true }
    ).exec();

    if (!trip) {
      return res.status(404).json({ message: 'Trip not found' });
    }

    return res.status(201).json(trip);
  } catch (err) {
    return res.status(400).json(err);
  }
};

// DELETE trip by code
const tripsDeleteTrip = async (req, res) => {
  try {
    const trip = await Trip.findOneAndDelete({ code: req.params.tripCode }).exec();

    if (!trip) {
      return res.status(404).json({ message: 'Trip not found' });
    }

    return res.status(200).json({ message: 'Trip deleted successfully' });
  } catch (err) {
    return res.status(400).json(err);
  }
};

module.exports = {
  tripsList,
  tripsFindCode,
  tripsAddTrip,
  tripsUpdateTrip,
  tripsDeleteTrip
};