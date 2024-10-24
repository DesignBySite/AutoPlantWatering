import React from 'react';
import Navbar from './navbar/navbar';
import Sidebar from './sidebar/sidebar';
import styles from './App.module.scss';
// import Chart from 'chart.js/auto'
import useSSEListener from './SSEListener/sseListener';

function App() {
  useSSEListener();


  return (
    <div className={styles.app__container}>
      <Navbar />
      <Sidebar />
      <section className={styles.app__main}>
        Main
        <div>
          <canvas id='sensor1'></canvas>
        </div>
      </section>
    </div>
  );
}

export default App;