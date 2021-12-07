--Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
----------------------------------------------------------------------------------
--Tool Version: Vivado v.2018.3 (win64) Build 2405991 Thu Dec  6 23:38:27 MST 2018
--Date        : Fri Dec  3 12:29:28 2021
--Host        : DESKTOP-S3B271S running 64-bit major release  (build 9200)
--Command     : generate_target design_1_wrapper.bd
--Design      : design_1_wrapper
--Purpose     : IP block netlist
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity design_1_wrapper is
  port (
    MISO : in STD_LOGIC;
    MISO1 : in STD_LOGIC;
    MISO2 : in STD_LOGIC;
    MOSI : out STD_LOGIC;
    MOSI1 : out STD_LOGIC;
    MOSI2 : out STD_LOGIC;
    SCLK : out STD_LOGIC;
    SCLK1 : out STD_LOGIC;
    SCLK2 : out STD_LOGIC;
    SS : out STD_LOGIC_VECTOR ( 0 to 0 );
    SS1 : out STD_LOGIC_VECTOR ( 0 to 0 );
    SS2 : out STD_LOGIC_VECTOR ( 0 to 0 );
    clk : in STD_LOGIC;
    reset : in STD_LOGIC
  );
end design_1_wrapper;

architecture STRUCTURE of design_1_wrapper is
  component design_1 is
  port (
    reset : in STD_LOGIC;
    clk : in STD_LOGIC;
    MOSI : out STD_LOGIC;
    MISO : in STD_LOGIC;
    SCLK : out STD_LOGIC;
    SS : out STD_LOGIC_VECTOR ( 0 to 0 );
    MOSI1 : out STD_LOGIC;
    MISO1 : in STD_LOGIC;
    SCLK1 : out STD_LOGIC;
    SS1 : out STD_LOGIC_VECTOR ( 0 to 0 );
    SS2 : out STD_LOGIC_VECTOR ( 0 to 0 );
    SCLK2 : out STD_LOGIC;
    MISO2 : in STD_LOGIC;
    MOSI2 : out STD_LOGIC
  );
  end component design_1;
begin
design_1_i: component design_1
     port map (
      MISO => MISO,
      MISO1 => MISO1,
      MISO2 => MISO2,
      MOSI => MOSI,
      MOSI1 => MOSI1,
      MOSI2 => MOSI2,
      SCLK => SCLK,
      SCLK1 => SCLK1,
      SCLK2 => SCLK2,
      SS(0) => SS(0),
      SS1(0) => SS1(0),
      SS2(0) => SS2(0),
      clk => clk,
      reset => reset
    );
end STRUCTURE;
