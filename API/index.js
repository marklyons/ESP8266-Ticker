const btcstats = require("btc-stats");
const express = require('express');
const curl = new (require('curl-request'))();
var cheerio = require("cheerio");
var app = express();

app.get('/ticker', function (req, res) {
  var jsonRet = [];
  btcstats.exchanges(["bitstamp"]);
    btcstats.avg(function(error, resp) {
      if (!error) {
        var priceStr = String(resp["price"]);
        var priceDouble = parseFloat(priceStr);
        
        var btcJson = {};
        btcJson["symbol"] = "BTC";
        btcJson["level"] = numberWithCommas(priceDouble);

        jsonRet.push(btcJson);

        curl.setHeaders([
          'user-agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.94 Safari/537.36'
        ])
        .get('https://money.cnn.com/data/premarket/')
        .then(({statusCode, body, headers}) => {
          var bodyStr = body.toString();
          $ = cheerio.load(bodyStr);
          
          var rows = $("#premkContent1 table tbody tr");
          for(var i = 0; i < rows.length; i += 5) {
            var currJson = {};
            var headRow = rows.eq(i);
            var levelRow = rows.eq(i+1);
    
            var currSymbol = headRow.find(".wsod_symbol").text();
            var level = levelRow.find(".wsod_aRight span").text();
            var changeSpans = headRow.find(".wsod_aRight span");
            var valueChange = $(changeSpans[0]).text();
            var percentChange = $(changeSpans[1]).text();
            var color = "green";
            if(valueChange.includes("-")) {
              color = "red";
            }
    
            currJson["symbol"] = currSymbol;
            currJson["level"] = level;
            currJson["value_change"] = valueChange;
            currJson["percent_change"] = percentChange;
            currJson["color"] = color;
    
            jsonRet.push(currJson);
          }
    
          
    
          res.send(jsonRet);
        })
        .catch((e) => {
          res.send(e);
        });
      }
    });  
});

app.get('/btcprice', function (req, res) {
  btcstats.exchanges(["bitstamp"]);
  btcstats.avg(function(error, resp) {
    if (!error) {
      var priceStr = String(resp["price"]);
      var priceDouble = parseFloat(priceStr);
      
      //var priceFinal = priceStr.replace("\"", "");
      res.send("BTC/USD $" + numberWithCommas(priceDouble));
    }
  });  
});

app.listen(process.env.PORT || 3000, () => console.log("server started"));

function numberWithCommas(x) {
  return x.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
}