import React, { useState, useEffect } from 'react';
import Commands from './components/Commands';
import Telemetry from './components/Telemetry';
import { sendCommand, getTelemetry, getMissionProgress, getBattery, getAltitude, getHeading } from './services/api';

function OperatorConsole() {
  const [telemetry, setTelemetry] = useState({
    latitude: 0,
    longitude: 0,
    relative_altitude_m: 0,
    heading_deg: 0,
    remaining_percent: 0,
    current: 0,
    total: 0,
  });

  const handleCommand = async (command) => {
    try {
      await sendCommand(command);
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
      <Commands onCommand={handleCommand} />
      <Telemetry telemetry={telemetry} />
    </div>
  );
}

export default OperatorConsole;