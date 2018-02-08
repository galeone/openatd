# OpenAT Daemon: crypto currency monitor and algorithmic trading bot

`openatd` is a daemon built using [OpenAT](https://github.com/galeone/openat).

You can configure the daemon for monitoring your preferred crypto currencies and trade them, for you, on different markets.

## Status

**Work in progress. The code in the repo won't compile on your machine. There are missing classes that are not public (yet)**

## Installation

The `openatd` service collects and store markets information about the specified currencies in the configuration file. Data is stored in the SQLite database at `~/.config/openat/db.db3`.

#### Configuration

Create a file named `config.json`. This file contains the information about the currencies you want to keep monitored, the pairs and the credential to use the available markets.
`config.json` should look like:

```json
{
    "monitor": {
        "currencies": [
            "btc",
            "xrp",
            "eth",
            "ltc",
            "xmr"
        ],
        "pairs": [
            [
                "btc",
                "usd"
            ],
            [
                "eth",
                "btc"
            ],
            [
                "eth",
                "usd"
            ],
            [
                "xmr",
                "usd"
            ]
        ],
        "period": 900
    },
    "markets": {
        "kraken": {
            "apiKey": "",
            "apiSecret": ""
        }
    },
    "strategies": {
        "xmr": {
            "usd": [
                {
                    "name": "DollarCostAveraging",
                    "params": {
                        "date": "16 17:30",
                        "buy": {
                            "quote": {
                                "fixed_amount": 100
                            }
                        }
                    }
                }
            ]
        },
        "btc": {
            "usd": [
                {
                    "name": "HODL",
                    "params": {}
                }
            ]
        }
    }
}
```

The available markets and exchanges are the one that OpenAT implements. The available implementations are visible here: https://github.com/galeone/openat/tree/master/include/at

#### Build

```bash
mkdir build
cd build
# Release
cmake ..
# For debug add -DCMAKE_BUILD_TYPE=Debug
make
cd ..
```

#### Install

```bash
# Install openatd under /usr/local/bin
sudo cp build/src/openatd /usr/local/bin/openatd
# Move the previous configuration file under ~/.config/openat/
mkdir -p ~/.config/openat/
cp config.json ~/.config/openat/
# Move the service file in the correct location
sudo cp misc/systemd/openatd@.service  /etc/systemd/system/openatd@.service
```

## Start

Enable the daemon on boot and start it with:

```bash
sudo systemctl enable openatd@$USER.service
sudo systemctl start openatd@$USER.service
```

<!--
## Auto Trader: strategies
TODO
-->
