#pragma once

#include "esphome/core/defines.h"  // does not see USE_ETHERNET_SPI otherwise
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/network/ip_address.h"
#include "esphome/components/spi/spi.h"

#ifdef USE_ESP32

#include "esp_eth.h"
#include "esp_eth_mac.h"
#include "esp_netif.h"

namespace esphome {
namespace ethernet {

enum EthernetType {
  ETHERNET_TYPE_LAN8720 = 0,
  ETHERNET_TYPE_RTL8201,
  ETHERNET_TYPE_DP83848,
  ETHERNET_TYPE_IP101,
  ETHERNET_TYPE_JL1101,
  ETHERNET_TYPE_W5500,
};

struct ManualIP {
  network::IPAddress static_ip;
  network::IPAddress gateway;
  network::IPAddress subnet;
  network::IPAddress dns1;  ///< The first DNS server. 0.0.0.0 for default.
  network::IPAddress dns2;  ///< The second DNS server. 0.0.0.0 for default.
};

enum class EthernetComponentState {
  STOPPED,
  CONNECTING,
  CONNECTED,
};

class EthernetComponent : public Component,
                          public spi::SPIDevice<spi::BIT_ORDER_LSB_FIRST, spi::CLOCK_POLARITY_LOW,
                                                spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_20MHZ> {
 public:
  EthernetComponent();
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;
  bool can_proceed() override;
  bool is_connected();

  // TODO: not defined here (due ethernet_component.h is imported before define.h) ???
#ifdef USE_ETHERNET_SPI
  void set_interrupt_pin(uint8_t interrupt_pin);
  void set_reset_pin(uint8_t reset_pin);
  void set_clock_speed(uint8_t clock_speed);
#else
  void set_phy_addr(uint8_t phy_addr);
  void set_power_pin(int power_pin);
  void set_mdc_pin(uint8_t mdc_pin);
  void set_mdio_pin(uint8_t mdio_pin);
  void set_clk_mode(emac_rmii_clock_mode_t clk_mode, emac_rmii_clock_gpio_t clk_gpio);
#endif

  void set_type(EthernetType type);
  void set_manual_ip(const ManualIP &manual_ip);

  network::IPAddress get_ip_address();
  std::string get_use_address() const;
  void set_use_address(const std::string &use_address);

 protected:
  static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
  static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

  void start_connect_();
  void dump_connect_params_();

  // TODO: not defined here (due ethernet_component.h is imported before define.h) ???
#ifdef USE_ETHERNET_SPI
  // uint8_t cs_pin_;
  uint8_t interrupt_pin_;
  int reset_pin_{-1};
  int phy_addr_spi_{-1};
  int clock_speed_{30 * 1000000};
#else
  uint8_t phy_addr_{0};
  int power_pin_{-1};
  uint8_t mdc_pin_{23};
  uint8_t mdio_pin_{18};
  emac_rmii_clock_mode_t clk_mode_{EMAC_CLK_EXT_IN};
  emac_rmii_clock_gpio_t clk_gpio_{EMAC_CLK_IN_GPIO};
#endif
  std::string use_address_;
  EthernetType type_{ETHERNET_TYPE_LAN8720};
  optional<ManualIP> manual_ip_{};

  bool started_{false};
  bool connected_{false};
  EthernetComponentState state_{EthernetComponentState::STOPPED};
  uint32_t connect_begin_;
  esp_netif_t *eth_netif_{nullptr};
  esp_eth_handle_t eth_handle_;
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern EthernetComponent *global_eth_component;
extern "C" esp_eth_phy_t *esp_eth_phy_new_jl1101(const eth_phy_config_t *config);

}  // namespace ethernet
}  // namespace esphome

#endif  // USE_ESP32
