const Commands = ({ onCommand, onFileSelect }) => {
  return (
    <div className="commands">
      <div>
        <label htmlFor="waypointFile">Select Waypoint File: </label>
        <input type="file" id="waypointFile" accept=".txt" onChange={onFileSelect} />
      </div>
      <button onClick={() => onCommand('start')}>Start</button>
      <button onClick={() => onCommand('pause')}>Pause</button>
      <button onClick={() => onCommand('resume')}>Resume</button>
      <button onClick={() => onCommand('abort')}>Abort</button>
    </div>
  );
};

export default Commands;