const Telemetry = ({ telemetry }) => {
  return (
    <div className="telemetry">
      <h2>Telemetry</h2>
      <p>Latitude: {telemetry.latitude}</p>
      <p>Longitude: {telemetry.longitude}</p>
      <p>Altitude: {telemetry.relative_altitude_m}m</p>
      <p>Heading: {telemetry.heading_deg}°</p>
      <p>Battery: {telemetry.remaining_percent}%</p>
      <p>Mission Progress: {telemetry.current} / {telemetry.total}</p>
    </div>
  );
};

export default Telemetry;