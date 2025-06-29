const Commands = ({ onCommand }) => {
  return (
    <div className="commands">
      <button onClick={() => onCommand('start')}>Start</button>
      <button onClick={() => onCommand('pause')}>Pause</button>
      <button onClick={() => onCommand('resume')}>Resume</button>
      <button onClick={() => onCommand('abort')}>Abort</button>
    </div>
  );
};

export default Commands;