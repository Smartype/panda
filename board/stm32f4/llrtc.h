void enable_bdomain_protection(void) {
  register_clear_bits(&(PWR->CR), PWR_CR_DBP);
}

void disable_bdomain_protection(void) {
  register_set_bits(&(PWR->CR), PWR_CR_DBP);
}

void rtc_init(void){
  uint32_t bdcr_opts = RCC_BDCR_RTCEN;
  uint32_t bdcr_mask = (RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL);
  if (current_board->has_rtc_battery) {
    bdcr_opts |= (RCC_BDCR_RTCSEL_0 | RCC_BDCR_LSEON);
    bdcr_mask |= (RCC_BDCR_LSEMOD | RCC_BDCR_LSEBYP | RCC_BDCR_LSEON);
  } else {
    bdcr_opts |= RCC_BDCR_RTCSEL_1;
    RCC->CSR |= RCC_CSR_LSION;
    while((RCC->CSR & RCC_CSR_LSIRDY) == 0){}
  }

  // Initialize RTC module and clock if not done already.
  if((RCC->BDCR & bdcr_mask) != bdcr_opts){
    print("Initializing RTC\n");
    // Reset backup domain
    register_set_bits(&(RCC->BDCR), RCC_BDCR_BDRST);

    // Disable write protection
    disable_bdomain_protection();

    // Clear backup domain reset
    register_clear_bits(&(RCC->BDCR), RCC_BDCR_BDRST);

    // Set RTC options
    register_set(&(RCC->BDCR), bdcr_opts, bdcr_mask);

    // Enable write protection
    enable_bdomain_protection();
  }

  // Disable write protection
  disable_bdomain_protection();
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  // Enable initialization mode
  register_set_bits(&(RTC->ISR), RTC_ISR_INIT);
  while((RTC->ISR & RTC_ISR_INITF) == 0){}

  // 33815.10509769589Hz
  register_set(&(RTC->PRER), 263 << RTC_PRER_PREDIV_S_Pos, RTC_PRER_PREDIV_S);

  // Disable initalization mode
  register_clear_bits(&(RTC->ISR), RTC_ISR_INIT);

  // Wait for synchronization
  while((RTC->ISR & RTC_ISR_RSF) == 0){}

  // Re-enable write protection
  RTC->WPR = 0x00;
  enable_bdomain_protection();
}

void rtc_wakeup_init(void) {
  EXTI->IMR  |=  EXTI_IMR_MR22;
  EXTI->RTSR |=  EXTI_RTSR_TR22; // rising edge
  EXTI->FTSR &=  ~EXTI_FTSR_TR22; // falling edge

  NVIC_DisableIRQ(RTC_WKUP_IRQn);

  // Disable write protection
  disable_bdomain_protection();
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  RTC->CR &= ~RTC_CR_WUTE;
  while((RTC->ISR & RTC_ISR_WUTWF) == 0){}

  RTC->CR &= ~RTC_CR_WUTIE;
  RTC->ISR &= ~RTC_ISR_WUTF;
  //PWR->CR |= PWR_CR_CWUF;

  RTC->WUTR = DEEPSLEEP_WAKEUP_DELAY;
  // Wakeup timer interrupt enable, wakeup timer enable, select 1Hz rate
  RTC->CR |= RTC_CR_WUTE | RTC_CR_WUTIE | RTC_CR_WUCKSEL_2;

  // Re-enable write protection
  RTC->WPR = 0x00;
  enable_bdomain_protection();

  NVIC_EnableIRQ(RTC_WKUP_IRQn);
}
