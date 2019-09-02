var btcstats = require("btc-stats");

//if this line isn't specified, it runs the avg function across all exchanges, not just these 3
btcstats.exchanges(["bitfinex", "bitstamp", "okcoin"]);

//Example print the average price across 3 exchanges (bitfinex, bitstamp, okcoin)
btcstats.avg(function(error, resp) {
	if (!error) {
		console.log(resp);
	}
});  