import styles from './sidebar.module.scss';

const Sidebar = () => {
  const handleOnclick = () => {
    console.log('clicked');
  }
  return(
    <aside className={styles.sidebar__container}>
      <div className={styles['sidebar__inner-style']}>
        <label>Tent 1</label>
        <ul>
          <li className={styles['sidebar__list-item']}>
            <button type='button' onClick={handleOnclick} className={styles.sensor__button}>Sensor 1</button>
          </li>
        </ul>
      </div>
    </aside>
  )
}

export default Sidebar;