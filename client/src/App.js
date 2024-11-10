import React, { useEffect, useState } from 'react';
import Navbar from './navbar/navbar';
import Sidebar from './sidebar/sidebar';
import styles from './App.module.scss';
import useSSEListener from './SSEListener/sseListener';
import useSensorStore from './contextStore/sensorStore';
import LineChart from './chart/chart';

function App() {
  useSSEListener();
  const [loading, setLoading] = useState(false)
  const getSensors = async() => {
    const data = await fetch('http://localhost:3050/sensors');
    const results = await data.json();
    await useSensorStore.getState().initialSensorDataImport(results);
    setLoading(useSensorStore.getState().initialLoad);
  }
  useEffect(() => {
    getSensors();
  },[useSensorStore.getState().sensors]);

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
        {loading === true ?
        <div className={styles['app__chart-container']}>
          {charts()}
        </div>
        : null
      }
      </section>
    </div>
  );
}

export default App;