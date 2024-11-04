import 'chartjs-adapter-date-fns';
import Chart from 'chart.js/auto';
import { useEffect, useRef, useState } from 'react';
import useSensorStore from '../contextStore/sensorStore';
import styles from './chart.module.scss'

const LineChart = ({ number }) => {
  const [dedupedData, setDedupedData] = useState()
  const chartRef = useRef(null);
  const chartInstance = useRef(null);
  const sensorInfo = useSensorStore.getState().getSensorInfo(number);
  
  useEffect(() => {
    if (!sensorInfo) {
      return;
    }
    setDedupedData([...new Set(sensorInfo.map(JSON.stringify))].map(JSON.parse));
  }, [sensorInfo])

  useEffect(() => {
    if (!dedupedData) {
      return;
    }

    if (chartInstance.current) {
      chartInstance.current.destroy();
    }

    // Convert date_time to day numbers from 0 to 30
    const dates = dedupedData.map((i) => new Date(i.date_time));
    const earliestDate = new Date(Math.min(...dates));

    const dayNumbers = dates.map((date) => {
      const dayDiff = (date - earliestDate) / (1000 * 60 * 60 * 24); // difference in days
      return dayDiff;
    });

    chartInstance.current = new Chart(chartRef.current, {
      type: 'line',
      data: {
        labels: dayNumbers, // X-axis labels as day numbers
        datasets: [
          {
            label: 'Moisture Readings',
            data: dedupedData.map((i) => i.moisture), // Y-axis data as moisture readings
            borderColor: 'rgba(75, 192, 192, 1)', // Line color
            backgroundColor: 'rgba(75, 192, 192, 0.2)', // Fill color under the line
            fill: true, // Enable fill under the line
            tension: 0.4, // Line tension for smoothness
          },
        ],
      },
      options: {
        responsive: true,
        maintainAspectRatio: true,
        plugins: {
          title: {
            display: true,
            text: `Sensor Number ${number}`, // Main chart title
            font: {
              size: 18,
            },
          },
        },
        scales: {
          x: {
            type: 'linear', // Use linear scale for x-axis
            min: 0,
            max: 30,
            title: {
              display: true,
              text: 'Days',
              font: {
                size: 14,
              },
            },
          },
          y: {
            beginAtZero: true,
            min: 0,
            max: 100,
            title: {
              display: true,
              text: 'Moisture (%)',
              font: {
                size: 14,
              },
            },
          },
        },
      },
    });

    return () => {
      if (chartInstance.current) {
        chartInstance.current.destroy();
      }
    };
  }, );

  return (
    <canvas
      ref={chartRef}
      id={`sensor${number}`}
      className={styles['single-chart']}
      style={{ backgroundColor: 'white' }} // Set canvas background to white
    ></canvas>
  );
};

export default LineChart;
