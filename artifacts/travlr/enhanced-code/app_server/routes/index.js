var express = require('express');
var router = express.Router();
var ctrlMain = require('../controllers/main');

/* GET home page */
router.get('/', ctrlMain.index);

/* GET travel page */
router.get('/travel', ctrlMain.travel);

module.exports = router;