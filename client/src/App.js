import React, { useEffect } from 'react';
import Navbar from './navbar/navbar';
import Sidebar from './sidebar/sidebar';
import styles from './App.module.scss';
import useSSEListener from './SSEListener/sseListener';
import useSensorStore from './contextStore/sensorStore';
import LineChart from './chart/chart';

function App() {
  useSSEListener();
  
  const getSensors = async() => {
    const data = await fetch('http://localhost:3050/sensors');
    const results = await data.json();
    useSensorStore.getState().initialSensorDataImport(results);
  }
  useEffect(() => {
    getSensors();
  },[]);

  const charts = () => {
    const array = []
    for (let index = 0; index < 8; index++) {
      array.push(<LineChart number={index} />)
    }
    return array;
  }

  return (
    <div className={styles.app__container}>
      <Navbar />
      <Sidebar />
      <section className={styles.app__main}>
        <div className={styles['app__chart-container']}>
          {charts()}
        </div>
      </section>
    </div>
  );
}

export default App;