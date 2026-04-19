const express = require("express");
const cors = require("cors");
const { spawn } = require("child_process");

const app = express();
app.use(cors());
app.use(express.json());

app.post("/debate", (req, res) => {
  const theme = (req.body.theme || "").trim();

  const proc = spawn("./debate_cli", [], {
    cwd: process.cwd()
  });

  proc.stdout.setEncoding("utf8");
  proc.stderr.setEncoding("utf8");

  res.setHeader("Content-Type", "text/plain; charset=utf-8");

  proc.stdin.write(theme + "\n");
  proc.stdin.end();

  proc.stdout.on("data", (data) => {
    res.write(data);
  });

  proc.stderr.on("data", (data) => {
    res.write("[server error] " + data);
  });

  proc.on("close", () => {
    res.end();
  });
});

app.listen(3000, () => {
  console.log("server running: http://localhost:3000");
});