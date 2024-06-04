const fs = require("fs")
const path = require("path")

const bins = [
  { lower: -200, upper: -50 },
  { lower: -50, upper: -40 },
  { lower: -30, upper: -20 },
  { lower: -20, upper: -10 },
  { lower: -10, upper: 0 },
  { lower: 0, upper: 5 },
  { lower: 5, upper: 10 },
  { lower: 10, upper: 15 },
  { lower: 15, upper: 20 },
  { lower: 20, upper: 25 },
  { lower: 25, upper: 30 },
  { lower: 30, upper: 35 },
  { lower: 35, upper: 40 },
  { lower: 40, upper: 50 },
  { lower: 50, upper: 200 },
]

const f = async () => {
  const res = await fetch("https://wowapi.metweb.ie/getObservations/dry_bulb?date=02%2F06%2F2024&time=20&timePoint=-1&mapFilterTags=&showWowData=true&showOfficialData=true&showRegisteredSites=false");
  const data = JSON.parse(await new Response(res.body).text())

  const lines = data.features.map(feat => {
    const temp = feat.properties.primary.dt
    if (temp == undefined) return "";
    const bin = bins.findIndex(x => x.lower <= temp && temp < x.upper)
    return `${feat.geometry.coordinates[0]};${feat.geometry.coordinates[1]};${bin}`
  })

  fs.writeFileSync(path.resolve("../data/temperature/data.txt"), lines.filter(x => x.length).join("\n"))
}

f();