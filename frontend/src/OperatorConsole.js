import React, { useState, useEffect } from 'react';
import Commands from './components/Commands';
import Telemetry from './components/Telemetry';
import Map from './components/Map';
import { sendCommand, getTelemetry, getMissionProgress, getBattery, getAltitude, getHeading } from './services/api';

function OperatorConsole() {
  const [telemetry, setTelemetry] = useState({
    latitude: 47.397742,
    longitude: 8.545594,
    relative_altitude_m: 0,
    heading_deg: 0,
    remaining_percent: 0,
    current: 0,
    total: 0,
  });

  const [waypoints, setWaypoints] = useState([]);
  const [waypointFileContent, setWaypointFileContent] = useState('');

  const handleFileSelect = (event) => {
    const file = event.target.files[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e) => {
        const text = e.target.result;
        setWaypointFileContent(text);
        const parsedWaypoints = text.trim().split('\n').map(line => {
          const [lat, lon] = line.split(',');
          return [parseFloat(lat), parseFloat(lon)];
        });
        setWaypoints(parsedWaypoints);
      };
      reader.readAsText(file);
    }
  };

  const handleCommand = async (command) => {
    try {
      if (command === 'start') {
        if (!waypointFileContent) {
          alert('Please select a waypoint file first!');
          return;
        }
        await sendCommand('start', waypointFileContent);
      } else {
        await sendCommand(command);
      }
      console.log(`Command sent: ${command}`);
    } catch (error) {
      console.error(`Error sending command: ${command}`, error);
    }
  };

  useEffect(() => {
    const fetchTelemetry = async () => {
      try {
        const [
          telemetryRes,
          missionProgressRes,
          batteryRes,
          altitudeRes,
          headingRes,
        ] = await Promise.all([
          getTelemetry(),
          getMissionProgress(),
          getBattery(),
          getAltitude(),
          getHeading(),
        ]);

        setTelemetry({
          latitude: telemetryRes.data.latitude,
          longitude: telemetryRes.data.longitude,
          relative_altitude_m: altitudeRes.data.relative_altitude_m,
          heading_deg: headingRes.data.heading_deg,
          remaining_percent: batteryRes.data.remaining_percent,
          current: missionProgressRes.data.current,
          total: missionProgressRes.data.total,
        });
      } catch (error) {
        console.error("Error fetching telemetry:", error);
      }
    };

    const interval = setInterval(fetchTelemetry, 1000); // Fetch every second

    return () => clearInterval(interval);
  }, []);

  return (
    <div>
      <h1>Freefly Foam Dispenser</h1>
      <div className="main-container">
        <div className="left-panel">
          <Commands onCommand={handleCommand} onFileSelect={handleFileSelect}/>
          <Telemetry telemetry={telemetry} />
        </div>
        <div className="right-panel">
          <Map position={telemetry} waypoints={waypoints} />
        </div>
      </div>
    </div>
  );
}

export default OperatorConsole;