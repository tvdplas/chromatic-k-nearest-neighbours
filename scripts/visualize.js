const {Image} = require('imagescript');
const fs = require('fs')
const path = require('path')

const NAME = "temperature-02-06-2024"
const WIDTH = 3600
const HEIGHT = 1800
const MIN_X_VAL = -180
const MAX_X_VAL = 180
const MIN_Y_VAL = -90
const MAX_Y_VAL = 90

const run = async () => {
    const pointsCsv = fs.readFileSync(path.resolve(`../data/${NAME}.points`)).toString()
    const points = pointsCsv.split("\n").map(line => {
      const items = line.split(";")
      return { 
        x: parseFloat(items[0]),
        y: parseFloat(items[1]),
        c: parseInt(items[2]),
      }
    })

    const colorsCount = Math.max(...points.map(p => p.c)) + 1
    const colors = Array.from({length: colorsCount + 1}).map(_ => {
      const color = Math.random() * Math.pow(2, 24)
      return color << 8 + 0xFF;
    })

    const image = new Image(WIDTH, HEIGHT);
    image.drawBox(0, 0, WIDTH, HEIGHT, 0xFFFFFFFF);

    points.forEach( point => 
      image.drawBox(
        Math.round((point.x - MIN_X_VAL) / (MAX_X_VAL - MIN_X_VAL) * WIDTH),
        HEIGHT - Math.round((point.y - MIN_Y_VAL) / (MAX_Y_VAL - MIN_Y_VAL) * HEIGHT),
        4, 4, colors[point.c]
      )
    )
    
    const encoded = await image.encode()

    await fs.writeFileSync(`../figs/renders/${NAME}.png`, encoded);
}

run();