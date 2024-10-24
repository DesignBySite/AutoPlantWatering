import styles from './navbar.module.scss';

const Navbar = () => {
  return(
    <nav className={styles.nav}>
      <ul className={styles.nav__container}>
        <li className={styles.nav__item}><a className={styles['nav__link-style']} href='#'>Home</a></li>
        <li className={styles.nav__item}><a className={styles['nav__link-style']} href='#'>About</a></li>
        <li className={styles.nav__item}><a className={styles['nav__link-style']} href='#'>Contact</a></li>
      </ul>
    </nav>
  )
}

export default Navbar;