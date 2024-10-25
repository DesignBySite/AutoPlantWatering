import React, { useEffect } from 'react';
import Navbar from './navbar/navbar';
import Sidebar from './sidebar/sidebar';
import styles from './App.module.scss';
// import Chart from 'chart.js/auto'
import useSSEListener from './SSEListener/sseListener';
import useSensorStore from './contextStore/sensorStore';

function App() {
  useSSEListener();

  const getSensors = async() => {
    const data = await fetch('http://localhost:3050/sensors');
    const results = await data.json();
    useSensorStore.getState().initialSensorDataImport(results);
  }
  useEffect(() => {
    getSensors();
  });

  const handleGetSensorNumber = (number) => {
    const sensors = useSensorStore.getState().sensors;
    console.log(sensors[number]);
  }

  return (
    <div className={styles.app__container}>
      <Navbar />
      <Sidebar />
      <section className={styles.app__main}>
        Main
        <button type='button' onClick={() => handleGetSensorNumber(0)}>Get Sensors</button>
        <div>
          <canvas id='sensor1'></canvas>
        </div>
      </section>
    </div>
  );
}

export default App;