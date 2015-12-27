#include <stdint.h>
#include "telemetrysimu.h"
#include "ui_telemetrysimu.h"
#include "simulatorinterface.h"
#include "radio/src/telemetry/frsky.h"

TelemetrySimulator::TelemetrySimulator(QWidget * parent, SimulatorInterface * simulator):
  QDialog(parent),
  ui(new Ui::TelemetrySimulator),
  simulator(simulator)
{
  ui->setupUi(this);

  QPoint dialogCenter = mapToGlobal(rect().center());
  QPoint parentWindowCenter = parent->window()->mapToGlobal(
    parent->window()->rect().center());
  move(parentWindowCenter - dialogCenter);

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(onTimerEvent()));

  logTimer = new QTimer(this);
  connect(logTimer, SIGNAL(timeout()), this, SLOT(onLogTimerEvent()));

  connect(ui->Simulate, SIGNAL(clicked(bool)), this, SLOT(onSimulateToggled(bool)));
  connect(ui->loadLogFile, SIGNAL(released()), this, SLOT(onLoadLogFile()));
  connect(ui->play, SIGNAL(released()), this, SLOT(onPlay()));
  connect(ui->rewind, SIGNAL(clicked()), this, SLOT(onRewind()));
  connect(ui->stepForward, SIGNAL(clicked()), this, SLOT(onStepForward()));
  connect(ui->stepBack, SIGNAL(clicked()), this, SLOT(onStepBack()));
  connect(ui->stop, SIGNAL(clicked()), this, SLOT(onStop()));
  connect(ui->positionIndicator, SIGNAL(valueChanged(int)), this, SLOT(onPositionIndicatorChanged(int)));
  connect(ui->replayRate, SIGNAL(valueChanged(int)), this, SLOT(onReplayRateChanged(int)));

  ui->A1->setSpecialValueText(" ");
  ui->A2->setSpecialValueText(" ");
  ui->A3->setSpecialValueText(" ");
  ui->A4->setSpecialValueText(" ");
  ui->rpm->setSpecialValueText(" ");
  ui->fuel->setSpecialValueText(" ");

  ui->rxbt_ratio->setEnabled(false);
  ui->A1_ratio->setEnabled(false);
  ui->A2_ratio->setEnabled(false);

  logPlayback = new LogPlaybackController(ui);
}

TelemetrySimulator::~TelemetrySimulator()
{
  timer->stop();
  logTimer->stop();
  delete ui;
}

void TelemetrySimulator::onSimulateToggled(bool isChecked)
{
  if (isChecked) {
    timer->start(10);
  }
  else {
    timer->stop();
  }
}


void TelemetrySimulator::onLogTimerEvent()
{
  logPlayback->stepForward(false);
}


void TelemetrySimulator::onTimerEvent()
{
  generateTelemetryFrame();
}

void TelemetrySimulator::onLoadLogFile()
{
  onStop(); // in case we are in playback mode
  logPlayback->loadLogFile();
}

void TelemetrySimulator::onPlay()
{
  if (logPlayback->isReady()) {
    logTimer->start(logPlayback->logFrequency * 1000 / SPEEDS[ui->replayRate->value()]);
    logPlayback->play();
  }
}

void TelemetrySimulator::onRewind()
{
  if (logPlayback->isReady()) {
    logTimer->stop();
    logPlayback->rewind();
  }
}

void TelemetrySimulator::onStepForward()
{
  if (logPlayback->isReady()) {
    logTimer->stop();
    logPlayback->stepForward(true);
  }
}

void TelemetrySimulator::onStepBack()
{
  if (logPlayback->isReady()) {
    logTimer->stop();
    logPlayback->stepBack();
  }
}

void TelemetrySimulator::onStop()
{
  if (logPlayback->isReady()) {
    logTimer->stop();
    logPlayback->stop();
  }
}

void TelemetrySimulator::onPositionIndicatorChanged(int value)
{
  if (logPlayback->isReady()) {
    logPlayback->updatePositionLabel(value);
    logPlayback->setUiDataValues();
  }
}

void TelemetrySimulator::onReplayRateChanged(int value)
{
  if (logTimer->isActive()) {
    logTimer->setInterval(logPlayback->logFrequency * 1000 / SPEEDS[ui->replayRate->value()]);
  }
}

void TelemetrySimulator::closeEvent(QCloseEvent *event)
{
  timer->stop();
  logTimer->stop();
  event->accept();
}

void TelemetrySimulator::showEvent(QShowEvent *event)
{

  ui->rxbt_inst->setText(QString::number(simulator->getSensorInstance(BATT_ID)));
  ui->rxbt_ratio->setValue(simulator->getSensorRatio(BATT_ID) / 10.0);
  ui->rssi_inst->setText(QString::number(simulator->getSensorInstance(RSSI_ID)));
  ui->swr_inst->setText(QString::number(simulator->getSensorInstance(SWR_ID)));
  ui->a1_inst->setText(QString::number(simulator->getSensorInstance(ADC1_ID)));
  ui->A1_ratio->setValue(simulator->getSensorRatio(ADC1_ID) / 10.0);
  ui->a2_inst->setText(QString::number(simulator->getSensorInstance(ADC2_ID)));
  ui->A2_ratio->setValue(simulator->getSensorRatio(ADC2_ID) / 10.0);
  ui->a3_inst->setText(QString::number(simulator->getSensorInstance(A3_FIRST_ID)));
  ui->a4_inst->setText(QString::number(simulator->getSensorInstance(A4_FIRST_ID)));
  ui->t1_inst->setText(QString::number(simulator->getSensorInstance(T1_FIRST_ID)));
  ui->t2_inst->setText(QString::number(simulator->getSensorInstance(T2_FIRST_ID)));
  ui->rpm_inst->setText(QString::number(simulator->getSensorInstance(RPM_FIRST_ID)));
  ui->fuel_inst->setText(QString::number(simulator->getSensorInstance(FUEL_FIRST_ID)));
  ui->aspd_inst->setText(QString::number(simulator->getSensorInstance(AIR_SPEED_FIRST_ID)));
  ui->vvspd_inst->setText(QString::number(simulator->getSensorInstance(VARIO_FIRST_ID)));
  ui->valt_inst->setText(QString::number(simulator->getSensorInstance(ALT_FIRST_ID)));
  ui->fasv_inst->setText(QString::number(simulator->getSensorInstance(VFAS_FIRST_ID)));
  ui->fasc_inst->setText(QString::number(simulator->getSensorInstance(CURR_FIRST_ID)));
  ui->cells_inst->setText(QString::number(simulator->getSensorInstance(CELLS_FIRST_ID)));
  ui->gpsa_inst->setText(QString::number(simulator->getSensorInstance(GPS_ALT_FIRST_ID)));
  ui->gpss_inst->setText(QString::number(simulator->getSensorInstance(GPS_SPEED_FIRST_ID)));
  ui->gpsc_inst->setText(QString::number(simulator->getSensorInstance(GPS_COURS_FIRST_ID)));
  ui->gpst_inst->setText(QString::number(simulator->getSensorInstance(GPS_TIME_DATE_FIRST_ID)));
  ui->gpsll_inst->setText(QString::number(simulator->getSensorInstance(GPS_LONG_LATI_FIRST_ID)));
  ui->accx_inst->setText(QString::number(simulator->getSensorInstance(ACCX_FIRST_ID)));
  ui->accy_inst->setText(QString::number(simulator->getSensorInstance(ACCY_FIRST_ID)));
  ui->accz_inst->setText(QString::number(simulator->getSensorInstance(ACCZ_FIRST_ID)));

  ui->Simulate->setChecked(true);
  onSimulateToggled(true); // not sure why this doesn't fire automatically
}

void setSportPacketCrc(uint8_t * packet)
{
  short crc = 0;
  for (int i=1; i<FRSKY_SPORT_PACKET_SIZE-1; i++) {
    crc += packet[i]; //0-1FF
    crc += crc >> 8; //0-100
    crc &= 0x00ff;
    crc += crc >> 8; //0-0FF
    crc &= 0x00ff;
  }
  packet[FRSKY_SPORT_PACKET_SIZE-1] = 0xFF - (crc & 0x00ff);
  //TRACE("crc set: %x", packet[FRSKY_SPORT_PACKET_SIZE-1]);
}

void generateSportPacket(uint8_t * packet, uint8_t dataId, uint8_t prim, uint16_t appId, uint32_t data)
{
  packet[0] = dataId;
  packet[1] = prim;
  *((uint16_t *)(packet+2)) = appId;
  *((int32_t *)(packet+4)) = data;
  setSportPacketCrc(packet);
}

void TelemetrySimulator::generateTelemetryFrame()
{
  static int item = 0;
  bool ok = true;
  uint8_t buffer[FRSKY_SPORT_PACKET_SIZE];
  static FlvssEmulator *flvss = new FlvssEmulator();
  static GPSEmulator *gps = new GPSEmulator();

  memset(buffer, 0, sizeof(buffer));

  switch (item++) {
    case 0:
#if defined(XJT_VERSION_ID)
      generateSportPacket(buffer, 1, DATA_FRAME, XJT_VERSION_ID, 11);
#endif
    break;

    case 1:
      if (ui->rxbt->text().length()) {
        generateSportPacket(buffer, ui->rxbt_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, BATT_ID, LIMIT<uint32_t>(0, ui->rxbt->value() * 255.0 / ui->rxbt_ratio->value(), 0xFFFFFFFF));
      }
      break;

    case 2:
      if (ui->Rssi->text().length())
        generateSportPacket(buffer, ui->rssi_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, RSSI_ID, LIMIT<uint32_t>(0, ui->Rssi->text().toInt(&ok, 0), 0xFF));
      break;

    case 3:
      if (ui->Swr->text().length())
        generateSportPacket(buffer, ui->swr_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, SWR_ID, LIMIT<uint32_t>(0, ui->Swr->text().toInt(&ok, 0), 0xFFFF));
      break;

    case 4:
      if (ui->A1->value() > 0)
        generateSportPacket(buffer, ui->a1_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, ADC1_ID, LIMIT<uint32_t>(0, ui->A1->value() * 255.0 / ui->A1_ratio->value(), 0xFF));
      break;

    case 5:
      if (ui->A2->value() > 0)
        generateSportPacket(buffer, ui->a2_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, ADC2_ID, LIMIT<uint32_t>(0, ui->A2->value() * 255.0 / ui->A2_ratio->value(), 0xFF));
      break;

    case 6:
      if (ui->A3->value() > 0)
        generateSportPacket(buffer, ui->a3_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, A3_FIRST_ID, LIMIT<uint32_t>(0, ui->A3->value() * 100.0, 0xFFFFFFFF));
      break;

    case 7:
      if (ui->A4->value() > 0)
        generateSportPacket(buffer, ui->a4_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, A4_FIRST_ID, LIMIT<uint32_t>(0, ui->A4->value() * 100.0, 0xFFFFFFFF));
      break;

    case 8:
      if (ui->T1->value() != 0)
        generateSportPacket(buffer, ui->t1_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, T1_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->T1->value(), 0x7FFFFFFF));
      break;

    case 9:
      if (ui->T2->value() != 0)
        generateSportPacket(buffer, ui->t2_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, T2_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->T2->value(), 0x7FFFFFFF));
      break;

    case 10:
      if (ui->rpm->value() > 0)
        generateSportPacket(buffer, ui->rpm_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, RPM_FIRST_ID, LIMIT<uint32_t>(0, ui->rpm->value(), 0xFFFF));
      break;

    case 11:
      if (ui->fuel->value() > 0)
        generateSportPacket(buffer, ui->fuel_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, FUEL_FIRST_ID, LIMIT<uint32_t>(0, ui->fuel->value(), 0xFFFF));
      break;

    case 12:
      if (ui->vspeed->value() != 0)
        generateSportPacket(buffer, ui->vvspd_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, VARIO_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->vspeed->value() * 100.0, 0x7FFFFFFF));
      break;

    case 13:
      if (ui->valt->value() != 0)
        generateSportPacket(buffer, ui->valt_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, ALT_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->valt->value() * 100.0, 0x7FFFFFFF));
      break;

    case 14:
      if (ui->vfas->value() != 0)
        generateSportPacket(buffer, ui->fasv_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, VFAS_FIRST_ID, LIMIT<uint32_t>(0, ui->vfas->value() * 100.0, 0xFFFFFFFF));
      break;

    case 15:
      if (ui->curr->value() != 0)
        generateSportPacket(buffer, ui->fasc_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, CURR_FIRST_ID, LIMIT<uint32_t>(0, ui->curr->value() * 10.0, 0xFFFFFFFF));
      break;

    case 16:
      double cellValues[FlvssEmulator::MAXCELLS];
      if (ui->cell1->value() > 0.009) { // ??? cell1 returning non-zero value when spin box is zero!
        cellValues[0] = ui->cell1->value();
        cellValues[1] = ui->cell2->value();
        cellValues[2] = ui->cell3->value();
        cellValues[3] = ui->cell4->value();
        cellValues[4] = ui->cell5->value();
        cellValues[5] = ui->cell6->value();
        generateSportPacket(buffer, ui->cells_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, CELLS_FIRST_ID, flvss->setAllCells_GetNextPair(cellValues));
      }
      else {
        cellValues[0] = 0;
        flvss->setAllCells_GetNextPair(cellValues);
      }
      break;

    case 17:
      if (ui->aspeed->value() > 0)
        generateSportPacket(buffer, ui->aspd_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, AIR_SPEED_FIRST_ID, LIMIT<uint32_t>(0, ui->aspeed->value() * 5.39957, 0xFFFFFFFF));
      break;

    case 18:
      if (ui->gps_alt->value() != 0) {
        gps->setGPSAltitude(ui->gps_alt->value());
        generateSportPacket(buffer, ui->gpsa_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, GPS_ALT_FIRST_ID, gps->getNextPacketData(GPS_ALT_FIRST_ID));
      }
      break;

    case 19:
      if (ui->gps_speed->value() > 0) {
        gps->setGPSSpeedKMH(ui->gps_speed->value());
        generateSportPacket(buffer, ui->gpss_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, GPS_SPEED_FIRST_ID, gps->getNextPacketData(GPS_SPEED_FIRST_ID));
      }
      break;

    case 20:
      if (ui->gps_course->value() != 0) {
        gps->setGPSCourse(ui->gps_course->value());
        generateSportPacket(buffer, ui->gpsc_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, GPS_COURS_FIRST_ID, gps->getNextPacketData(GPS_COURS_FIRST_ID));
      }
      break;

    case 21:
      if (ui->gps_time->text().length()) {
        gps->setGPSDateTime(ui->gps_time->text());
        generateSportPacket(buffer, ui->gpst_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, GPS_TIME_DATE_FIRST_ID, gps->getNextPacketData(GPS_TIME_DATE_FIRST_ID));
      }
      break;

    case 22:
      if (ui->gps_latlon->text().length()) {
        gps->setGPSLatLon(ui->gps_latlon->text());
        generateSportPacket(buffer, ui->gpsll_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, GPS_LONG_LATI_FIRST_ID, gps->getNextPacketData(GPS_LONG_LATI_FIRST_ID));
      }
      break;

    case 23:
        if (ui->accx->value() != 0)
          generateSportPacket(buffer, ui->accx_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, ACCX_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->accx->value() * 100.0, 0x7FFFFFFF));
        break;

    case 24:
      if (ui->accy->value() != 0)
        generateSportPacket(buffer, ui->accy_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, ACCY_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->accy->value() * 100.0, 0x7FFFFFFF));
      break;

    case 25:
      if (ui->accz->value() != 0)
        generateSportPacket(buffer, ui->accz_inst->text().toInt(&ok, 0) - 1, DATA_FRAME, ACCZ_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->accz->value() * 100.0, 0x7FFFFFFF));
      break;

    default:
      item = 0;
      return;
  }

  if (ok && buffer[0])
    simulator->sendTelemetry(buffer, FRSKY_SPORT_PACKET_SIZE);
  else
    onTimerEvent();
}

uint32_t TelemetrySimulator::FlvssEmulator::encodeCellPair(uint8_t cellNum, uint8_t firstCellNo, double cell1, double cell2)
{
  uint16_t cell1Data = cell1 * 500.0;
  uint16_t cell2Data = cell2 * 500.0;
  uint32_t cellData = 0;

  cellData = cell2Data & 0x0FFF;
  cellData <<= 12;
  cellData |= cell1Data & 0x0FFF;
  cellData <<= 4;
  cellData |= cellNum & 0x0F;
  cellData <<= 4;
  cellData |= firstCellNo & 0x0F;

  return cellData;
}

void TelemetrySimulator::FlvssEmulator::encodeAllCells()
{
  cellData1 = encodeCellPair(numCells, 0, cellFloats[0], cellFloats[1]);
  if (numCells > 2) cellData2 = encodeCellPair(numCells, 2, cellFloats[2], cellFloats[3]); else cellData2 = 0;
  if (numCells > 4) cellData3 = encodeCellPair(numCells, 4, cellFloats[4], cellFloats[5]); else cellData3 = 0;
}

void TelemetrySimulator::FlvssEmulator::splitIntoCells(double totalVolts)
{
  numCells = qFloor((totalVolts / 3.7) + .5);
  double avgVolts = totalVolts / numCells;
  double remainder = (totalVolts - (avgVolts * numCells));
    for (uint32_t i = 0; (i < numCells) && ( i < MAXCELLS); i++) {
    cellFloats[i] = avgVolts;
  }
  for (uint32_t i = numCells; i < MAXCELLS; i++) {
    cellFloats[i] = 0;
  }
  cellFloats[0] += remainder;
  numCells = numCells > MAXCELLS ? MAXCELLS : numCells; // force into valid cell count in case of input out of range
}

uint32_t TelemetrySimulator::FlvssEmulator::setAllCells_GetNextPair(double cellValues[6])
{
  numCells = 0;
  for (uint32_t i = 0; i < MAXCELLS; i++) {
    if ((i == 0) && (cellValues[0] > 4.2)) {
      splitIntoCells(cellValues[0]);
      break;
    }
    if (cellValues[i] > 0) {
      cellFloats[i] = cellValues[i];
      numCells++;
    }
    else {
      // zero marks the last cell
      for (uint32_t x = i; x < MAXCELLS; x++) {
        cellFloats[x] = 0;
      }
      break;
    }
  }

  // encode the double values into telemetry format
  encodeAllCells();

  // return the value for the current pair 
  uint32_t cellData = 0;
  if (nextCellNum >= numCells) {
    nextCellNum = 0;
  }
  switch (nextCellNum) {
  case 0:
    cellData = cellData1;
    break;
  case 2:
    cellData = cellData2;
    break;
  case 4:
    cellData = cellData3;
    break;
  }
  nextCellNum += 2;
  return cellData;
}

TelemetrySimulator::GPSEmulator::GPSEmulator()
{
  lat = 0;
  lon = 0;
  dt = QDateTime::currentDateTime();
  sendLat = true;
  sendDate = true;
}


uint32_t TelemetrySimulator::GPSEmulator::encodeLatLon(double latLon, bool isLat)
{
  uint32_t data = (uint32_t)((latLon < 0 ? -latLon : latLon) * 60 * 10000) & 0x3FFFFFFF;
  if (isLat == false) {
    data |= 0x80000000;
  }
  if (latLon < 0) {
    data |= 0x40000000;
  }
  return data;
}

uint32_t TelemetrySimulator::GPSEmulator::encodeDateTime(uint8_t yearOrHour, uint8_t monthOrMinute, uint8_t dayOrSecond, bool isDate)
{
  uint32_t data = yearOrHour;
  data <<= 8;
  data |= monthOrMinute;
  data <<= 8;
  data |= dayOrSecond;
  data <<= 8;
  if (isDate == true) {
    data |= 0xFF;
  }
  return data;
}

uint32_t TelemetrySimulator::GPSEmulator::getNextPacketData(uint32_t packetType)
{
  switch (packetType) {
  case GPS_LONG_LATI_FIRST_ID:
    sendLat = !sendLat;
    return sendLat ? encodeLatLon(lat, TRUE) : encodeLatLon(lon, FALSE);
    break;
  case GPS_TIME_DATE_FIRST_ID:
    sendDate = !sendDate;
    return sendDate ? encodeDateTime(dt.date().year() - 2000, dt.date().month(), dt.date().day(), true) : encodeDateTime(dt.time().hour(), dt.time().minute(), dt.time().second(), false);
    break;
  case GPS_ALT_FIRST_ID:
    return (uint32_t) (altitude * 100);
    break;
  case GPS_SPEED_FIRST_ID:
    return speedKNTS * 1000;
    break;
  case GPS_COURS_FIRST_ID:
    return course * 100;
    break;
  }
  return 0;
}

void TelemetrySimulator::GPSEmulator::setGPSDateTime(QString dateTime)
{
  dt = QDateTime::currentDateTime().toTimeSpec(Qt::UTC); // default to current systemtime
  if (!dateTime.startsWith('*')) {
    QString format("dd-MM-yyyy hh:mm:ss");
    dt = QDateTime::fromString(dateTime, format);
  }
}

void TelemetrySimulator::GPSEmulator::setGPSLatLon(QString latLon)
{
  QStringList coords = latLon.split(",");
  lat = 0.0;
  lon = 0.0;
  if (coords.length() > 1)
  {
    lat = coords[0].toDouble();
    lon = coords[1].toDouble();
  }
}

void TelemetrySimulator::GPSEmulator::setGPSCourse(double course)
{
  this->course = course;
}

void TelemetrySimulator::GPSEmulator::setGPSSpeedKMH(double speedKMH)
{
  this->speedKNTS = speedKMH * 0.539957;
}

void TelemetrySimulator::GPSEmulator::setGPSAltitude(double altitude)
{
  this->altitude = altitude;
}

TelemetrySimulator::LogPlaybackController::LogPlaybackController(Ui::TelemetrySimulator * ui)
{
  TelemetrySimulator::LogPlaybackController::ui = ui;
  stepping = false;
  // initialize the map - TODO: how should this be localized?
  colToFuncMap.clear();
  colToFuncMap.insert("RxBt(V)", RXBT_V);
  colToFuncMap.insert("RSSI(dB)", RSSI);
  colToFuncMap.insert("SWR", SWR);
  colToFuncMap.insert("A1", A1);
  colToFuncMap.insert("A1(V)", A1);
  colToFuncMap.insert("A2", A2);
  colToFuncMap.insert("A2(V)", A2);
  colToFuncMap.insert("A3", A3);
  colToFuncMap.insert("A3(V)", A3);
  colToFuncMap.insert("A4", A4);
  colToFuncMap.insert("A4(V)", A4);
  colToFuncMap.insert("Tmp1(@C)", T1_DEGC);
  colToFuncMap.insert("Tmp1(@F)", T1_DEGF);
  colToFuncMap.insert("Tmp2(@C)", T2_DEGC);
  colToFuncMap.insert("Tmp2(@F)", T2_DEGF);
  colToFuncMap.insert("RPM(rpm)", RPM);
  colToFuncMap.insert("Fuel(%)", FUEL);
  colToFuncMap.insert("VSpd(m/s)", VSPD_MS);
  colToFuncMap.insert("VSpd(f/s)", VSPD_FS);
  colToFuncMap.insert("Alt(ft)", ALT_FEET);
  colToFuncMap.insert("Alt(m)", ALT_METERS);
  colToFuncMap.insert("VFAS(V)", FASV);
  colToFuncMap.insert("Curr(A)", FASC);
  colToFuncMap.insert("Cels(gRe)", CELS_GRE);
  colToFuncMap.insert("ASpd(kts)", ASPD_KTS);
  colToFuncMap.insert("ASpd(kmh)", ASPD_KMH);
  colToFuncMap.insert("ASpd(mph)", ASPD_MPH);
  colToFuncMap.insert("GAlt(ft)", GALT_FEET);
  colToFuncMap.insert("GAlt(m)", GALT_METERS);
  colToFuncMap.insert("GSpd(kts)", GSPD_KNTS);
  colToFuncMap.insert("GSpd(kmh)", GSPD_KMH);
  colToFuncMap.insert("GSpd(mph)", GSPD_MPH);
  colToFuncMap.insert("Hdg(@)", GHDG_DEG);
  colToFuncMap.insert("Date", GDATE);
  colToFuncMap.insert("GPS", G_LATLON);
  colToFuncMap.insert("AccX(g)", ACCX);
  colToFuncMap.insert("AccY(g)", ACCY);
  colToFuncMap.insert("AccZ(g)", ACCZ);
  
  // ACCX Y and Z
}

QDateTime TelemetrySimulator::LogPlaybackController::parseTransmittterTimestamp(QString row)
{
  QStringList rowParts = row.simplified().split(',');
  if (rowParts.size() < 2) {
    return QDateTime();
  }
  QString datePart = rowParts[0];
  QString timePart = rowParts[1];
  QDateTime result;
  QString format("yyyy-MM-dd hh:mm:ss.zzz"); // assume this format
  // hour can be 'missing'
  if (timePart.count(":") < 2) {
    timePart = "00:" + timePart;
  }
  if (datePart.contains("/")) { // happens when csv is edited by Excel
    format = "M/d/yyyy hh:mm:ss.z";
  }
  return QDateTime::fromString(datePart + " " + timePart, format);
}

void TelemetrySimulator::LogPlaybackController::calcLogFrequency()
{
  // examine up to 20 rows to determine log frequency in seconds
  logFrequency = 25.5; // default value
  QDateTime lastTime;
  for (int i = 1; (i < 20) && (i < csvRecords.count()); i++)
  {
    QDateTime logTime = parseTransmittterTimestamp(csvRecords[i]);
    // ugh - no timespan in this Qt version
    double timeDiff = (logTime.toMSecsSinceEpoch() - lastTime.toMSecsSinceEpoch()) / 1000.0;
    if ((timeDiff > 0) && (timeDiff < logFrequency)) {
      logFrequency = timeDiff;
    }
    lastTime = logTime;
  }
}

bool TelemetrySimulator::LogPlaybackController::isReady()
{
  return csvRecords.count() > 1;
}

void TelemetrySimulator::LogPlaybackController::loadLogFile()
{
  // reset the playback ui
  ui->play->setEnabled(false);
  ui->rewind->setEnabled(false);
  ui->stepBack->setEnabled(false);
  ui->stepForward->setEnabled(false);
  ui->stop->setEnabled(false);
  ui->positionIndicator->setEnabled(false);
  ui->replayRate->setEnabled(false);
  ui->positionLabel->setText("Row #\nTimestamp");

  // clear existing data
  csvRecords.clear();

  QString logFileNameAndPath = QFileDialog::getOpenFileName(NULL, tr("Log File"), ".", tr("LOG Files (*.csv)"));
  QFileInfo fileInfo(logFileNameAndPath);
  QFile file(logFileNameAndPath);
  if (!file.open(QIODevice::ReadOnly)) {
    ui->logFileLabel->setText(tr("ERROR - invalid file"));
    return;
  }
  while (!file.atEnd()) {
    QByteArray line = file.readLine();
    csvRecords.append(line.simplified());
  }
  if (csvRecords.count() > 1) {
    columnNames.clear();
    QStringList keys = csvRecords[0].split(',');
    // override the first two column names
    keys[0] = "LogDate";
    keys[1] = "LogTime";
    Q_FOREACH(QString key, keys) {
      columnNames.append(key.simplified());
    }
    ui->play->setEnabled(true);
    ui->rewind->setEnabled(true);
    ui->stepBack->setEnabled(true);
    ui->stepForward->setEnabled(true);
    ui->stop->setEnabled(true);
    ui->positionIndicator->setEnabled(true);
    ui->replayRate->setEnabled(true);
    supportedCols.clear();
    recordIndex = 1;
    calcLogFrequency();
  }
  ui->logFileLabel->setText(QFileInfo(logFileNameAndPath).fileName());
  // iterate through all known mappings and add those that are used
  QMapIterator<QString, CONVERT_TYPE> it(colToFuncMap);
  while (it.hasNext()) {
    it.next();
    addColumnHash(it.key(), it.value());
  }
  rewind();
  return;
}

void TelemetrySimulator::LogPlaybackController::addColumnHash(QString key, CONVERT_TYPE functionIndex)
{
  DATA_TO_FUNC_XREF dfx;
  if (columnNames.contains(key)) {
    dfx.functionIndex = functionIndex;
    dfx.dataIndex = columnNames.indexOf(key);
    supportedCols.append(dfx);
  }
}

void TelemetrySimulator::LogPlaybackController::play()
{
}

void TelemetrySimulator::LogPlaybackController::stop()
{
}

void TelemetrySimulator::LogPlaybackController::rewind()
{
  stepping = true;
  recordIndex = 1;
  ui->stop->setChecked(true);
  updatePositionLabel(-1);
  setUiDataValues();
  stepping = false;
}

void TelemetrySimulator::LogPlaybackController::stepForward(bool focusOnStop)
{
  stepping = true;
  if (recordIndex < (csvRecords.count() - 1)) {
    recordIndex++;
    if (focusOnStop) {
      ui->stop->setChecked(true);
    }
    updatePositionLabel(-1);
    setUiDataValues();
  }
  else {
    rewind(); // always loop at the end
  }
  stepping = false;
}

void TelemetrySimulator::LogPlaybackController::stepBack()
{
  stepping = true;
  if (recordIndex > 1) {
    recordIndex--;
    ui->stop->setChecked(true);
    updatePositionLabel(-1);
    setUiDataValues();
  }
  stepping = false;
}

double TelemetrySimulator::LogPlaybackController::convertFeetToMeters(QString input)
{
  double meters100 = input.toDouble() * 0.3048;
  return qFloor(meters100 + .005);
}

QString TelemetrySimulator::LogPlaybackController::convertGPSDate(QString input)
{
  QStringList dateTime = input.simplified().split(' ');
  if (dateTime.size() < 2) {
    return ""; // invalid format
  }
  QStringList dateParts = dateTime[0].split('-'); // input as yy-mm-dd
  if (dateParts.size() < 3) {
    return ""; // invalid format
  }
  // output is dd-MM-yyyy hh:mm:ss
  QString localDateString = dateParts[2] + "-" + dateParts[1] + "-20" + dateParts[0] + " " + dateTime[1];
  QString format("dd-MM-yyyy hh:mm:ss");
  QDateTime utcDate = QDateTime::fromString(localDateString, format).toTimeSpec(Qt::UTC);
  return utcDate.toString(format);
}

double TelemetrySimulator::LogPlaybackController::convertDegMin(QString input)
{
  double fInput = input.mid(0, input.length() - 1).toDouble();
  double degrees = qFloor(fInput / 100.0);
  double minutes = fInput - (degrees * 100);
  int32_t sign = ((input.endsWith('E')) || (input.endsWith('N'))) ? 1 : -1;
  return (degrees + (minutes / 60)) * sign;
}

QString TelemetrySimulator::LogPlaybackController::convertGPS(QString input)
{
  // input format is DDmm.mmmmH DDDmm.mmmmH (longitude latitude - degrees (2 places) minutes (2 places) decimal minutes (4 places)) 
  QStringList lonLat = input.simplified().split(' ');
  if (lonLat.count() < 2) {
    return ""; // invalid format
  }
  double lon = convertDegMin(lonLat[0]);
  double lat = convertDegMin(lonLat[1]);
  return QString::number(lat) + ", " + QString::number(lon);
}

void TelemetrySimulator::LogPlaybackController::updatePositionLabel(int32_t percentage)
{
  if ((percentage > 0) && (!stepping)) {
    recordIndex = qFloor((double)csvRecords.count() / 100.0 * percentage);
    if (recordIndex == 0) {
      recordIndex = 1; // record 0 is column labels
    }
  }
  // format the transmitter date info
  QDateTime transmitterTimestamp = parseTransmittterTimestamp(csvRecords[recordIndex]);
  QString format("yyyy-MM-dd hh:mm:ss.z");
  ui->positionLabel->setText("Row " + QString::number(recordIndex) + " of " + QString::number(csvRecords.count() - 1)
              + "\n" + transmitterTimestamp.toString(format));
  if (percentage < 0) { // did we step past a threshold?
    uint32_t posPercent = (recordIndex / (double)(csvRecords.count() - 1)) * 100;
    ui->positionIndicator->setValue(posPercent);
  }
}

double TelemetrySimulator::LogPlaybackController::convertFahrenheitToCelsius(QString input)
{
  return (input.toDouble() - 32.0) * 0.5556;
}

void TelemetrySimulator::LogPlaybackController::setUiDataValues()
{
  QStringList columnData = csvRecords[recordIndex].split(',');
  Q_FOREACH(DATA_TO_FUNC_XREF info, supportedCols) {
    if (info.dataIndex < columnData.size()) {
      switch (info.functionIndex) {
      case RXBT_V:
        ui->rxbt->setValue(columnData[info.dataIndex].toDouble());
        break;
      case RSSI:
        ui->Rssi->setValue(columnData[info.dataIndex].toDouble());
        break;
      case SWR:
        ui->Swr->setValue(columnData[info.dataIndex].toDouble());
        break;
      case A1:
        ui->A1->setValue(columnData[info.dataIndex].toDouble());
        break;
      case A2:
        ui->A2->setValue(columnData[info.dataIndex].toDouble());
        break;
      case A3:
        ui->A3->setValue(columnData[info.dataIndex].toDouble());
        break;
      case A4:
        ui->A4->setValue(columnData[info.dataIndex].toDouble());
        break;
      case T1_DEGC:
        ui->T1->setValue(columnData[info.dataIndex].toDouble());
        break;
      case T2_DEGC:
        ui->T2->setValue(columnData[info.dataIndex].toDouble());
        break;
      case T1_DEGF:
        ui->T1->setValue(convertFahrenheitToCelsius(columnData[info.dataIndex]));
        break;
      case T2_DEGF:
        ui->T2->setValue(convertFahrenheitToCelsius(columnData[info.dataIndex]));
        break;
      case RPM:
        ui->rpm->setValue(columnData[info.dataIndex].toDouble());
        break;
      case FUEL:
        ui->fuel->setValue(columnData[info.dataIndex].toDouble());
        break;
      case VSPD_MS:
        ui->vspeed->setValue(columnData[info.dataIndex].toDouble());
        break;
      case VSPD_FS:
        ui->vspeed->setValue(columnData[info.dataIndex].toDouble() * 0.3048);
        break;
      case ALT_FEET:
        ui->valt->setValue(convertFeetToMeters(columnData[info.dataIndex]));
        break;
      case ALT_METERS:
        ui->valt->setValue(columnData[info.dataIndex].toDouble());
        break;
      case FASV:
        ui->vfas->setValue(columnData[info.dataIndex].toDouble());
        break;
      case FASC:
        ui->curr->setValue(columnData[info.dataIndex].toDouble());
        break;
      case CELS_GRE:
        ui->cell1->setValue(columnData[info.dataIndex].toDouble());
        break;
      case ASPD_KTS:
        ui->aspeed->setValue(columnData[info.dataIndex].toDouble() * 1.8520008892119);
        break;
      case ASPD_KMH:
        ui->aspeed->setValue(columnData[info.dataIndex].toDouble());
        break;
      case ASPD_MPH:
        ui->aspeed->setValue(columnData[info.dataIndex].toDouble() * 1.60934);
        break;
      case GALT_FEET:
        ui->gps_alt->setValue(convertFeetToMeters(columnData[info.dataIndex]));
        break;
      case GALT_METERS:
        ui->gps_alt->setValue(columnData[info.dataIndex].toDouble());
        break;
      case GSPD_KNTS:
        ui->gps_speed->setValue(columnData[info.dataIndex].toDouble() * 1.852);
        break;
      case GSPD_KMH:
        ui->gps_speed->setValue(columnData[info.dataIndex].toDouble());
        break;
      case GSPD_MPH:
        ui->gps_speed->setValue(columnData[info.dataIndex].toDouble() * 1.60934);
        break;
      case GHDG_DEG:
        ui->gps_course->setValue(columnData[info.dataIndex].toDouble());
        break;
      case GDATE:
        ui->gps_time->setText(convertGPSDate(columnData[info.dataIndex]));
        break;
      case G_LATLON:
        ui->gps_latlon->setText(convertGPS(columnData[info.dataIndex]));
        break;
      case ACCX:
        ui->accx->setValue(columnData[info.dataIndex].toDouble());
        break;
      case ACCY:
        ui->accy->setValue(columnData[info.dataIndex].toDouble());
        break;
      case ACCZ:
        ui->accz->setValue(columnData[info.dataIndex].toDouble());
        break;
      }
    }
    else {
      // file is corrupt - shut down with open logs, or log format changed mid-day
    }
  }
}
