const express = require('express');
const router = express.Router();
const ctrlTrips = require('../controllers/trips');
const authController = require('../controllers/authentication');
const jwt = require('jsonwebtoken');

// Method to authenticate our JWT
function authenticateJWT(req, res, next) {
  const authHeader = req.headers['authorization'];

  if (authHeader == null) {
    console.log('Auth Header Required but NOT PRESENT!');
    return res.sendStatus(401);
  }

  const headers = authHeader.split(' ');
  if (headers.length < 2) {
    console.log('Not enough tokens in Auth Header: ' + headers.length);
    return res.sendStatus(401);
  }

  const token = headers[1];

  if (token == null) {
    console.log('Null Bearer Token');
    return res.sendStatus(401);
  }

  jwt.verify(token, process.env.JWT_SECRET, (err, verified) => {
    if (err) {
      return res.status(401).json('Token Validation Error!');
    }

    req.auth = verified;
    next();
  });
}

// Method to require admin role
function requireAdmin(req, res, next) {
  if (!req.auth || req.auth.role !== 'admin') {
    return res.status(403).json({
      message: 'Admin role required'
    });
  }

  next();
}

router.post('/register', authController.register);
router.post('/login', authController.login);

router.get('/trips', ctrlTrips.tripsList);
router.get('/trips/:tripCode', ctrlTrips.tripsFindCode);

router.post('/trips', authenticateJWT, ctrlTrips.tripsAddTrip);
router.put('/trips/:tripCode', authenticateJWT, ctrlTrips.tripsUpdateTrip);
router.delete('/trips/:tripCode', authenticateJWT, requireAdmin, ctrlTrips.tripsDeleteTrip);

module.exports = router;