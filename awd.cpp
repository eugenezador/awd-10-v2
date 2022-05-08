#include "awd.h"
#include "ui_awd.h"

awd::awd(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::awd)
{
    ui->setupUi(this);

    serial = new QSerialPort(this);

//чтение доступных портов при запуске
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
       {
           ui->portName->addItem(serialPortInfo.portName());
       }

// параметры порта
    currentPortName = ui->portName->currentText();

    serial->setPortName(currentPortName);

    serial->open(QIODevice::ReadWrite);
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    qDebug() << "PORT: " << serial->portName();

    connect(ui->portName, &QComboBox::currentTextChanged, this, &awd::serial_port_properties);

// Параметры
    set_labels_array();
    set_param_26_items();

// Режим
    set_mode_items();
    set_mode_connections();

// Статус
    status_no_edit();

// График
    plot_settings();

    cursorText = new QCPItemText(ui->plot);
    hLine = new QCPItemLine(ui->plot);
    vLine = new QCPItemLine(ui->plot);

    connect(ui->plot, &QCustomPlot::mouseMove, this, &awd::slotMouseMove);//отображение координат

    // Инициализируем трассировщик
    //tracer = new QCPItemTracer(ui->plot);

    //tracer->setGraph(ui->plot->graph(0));

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &awd::slot_for_new_point);// соединение для создания новой точки скорости


    timer->start(400);
    ui->spinBox_period->setValue(400);



    connect(ui->speed_horizontalSlider, &QSlider::valueChanged, this, &awd::speed_SliderValueChanged );

    //отправление эхо команды каждые 10 мс для постоянного чтения статуса если нет действий
    exo_timer = new QTimer(this);
    connect(exo_timer, &QTimer::timeout, this, &awd::send_exo);
    exo_timer->start(400);

    time.start();// старт времени графика
}

awd::~awd()
{
    delete timer;
    delete exo_timer;
    //delete tracer;
    serial->close();
    delete serial;
    delete ui;
}

void awd::serial_port_properties(const QString &text)
{

    bool currentPortNameChanged = false;

    if (currentPortName != text) {
        currentPortName = text;
        currentPortNameChanged = true;
       } else {
           currentPortNameChanged = false;
       }

    if (currentPortNameChanged) {

        serial->close();

        serial->setPortName(currentPortName);

        serial->open(QIODevice::ReadWrite);
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);
     }

     qDebug() << "PORT: " << text << " : " << serial->portName();

     ui->address_edit->setEnabled(1);
}

unsigned char awd::checkSumm(const unsigned char array[8])
{
    unsigned char summ = 0;
    for(int i = 0; i < 8; i++){
        summ -= array[i];
    }
    return summ;
}

void awd::send_exo()
{
    command[1] = 0xf0;
    command[2] = 0x00;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::command_formation(const QString &value,const int &param_num)
{

    command[1] = 0x78;
    command[2] = param_num;
    command[3] = 0x00;
    command[4] = (value.toInt() >> 8) & 0xFF;
    command[5] = value.toInt() & 0xFF;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::command_formation(int param_num)
{
    command[1] = 0x87;
    command[2] = param_num;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::set_param_26_items()
{
    ui->param_new_value_26->addItem("16");
    ui->param_new_value_26->addItem("1");
    ui->param_new_value_26->addItem("2");
    ui->param_new_value_26->addItem("4");
    ui->param_new_value_26->addItem("8");
}



void awd::set_labels_array()
{
    param_current_value_array[0] = ui->param_current_value_0;
    param_current_value_array[1] = ui->param_current_value_1;
    param_current_value_array[2] = ui->param_current_value_2;
    param_current_value_array[3] = ui->param_current_value_3;
    param_current_value_array[4] = ui->param_current_value_4;

    param_current_value_array[13]= ui->param_current_value_13;
    param_current_value_array[14] = ui->param_current_value_14;
    param_current_value_array[15] = ui->param_current_value_15;
    param_current_value_array[16] = ui->param_current_value_16;
    param_current_value_array[17] = ui->param_current_value_17;

    param_current_value_array[21] = ui->param_current_value_21;
    param_current_value_array[22] = ui->param_current_value_22;
    param_current_value_array[23] = ui->param_current_value_23;
    param_current_value_array[24] = ui->param_current_value_24;
    param_current_value_array[25] = ui->param_current_value_25;
    param_current_value_array[26] = ui->param_current_value_26;
    param_current_value_array[27] = ui->param_current_value_27;
    param_current_value_array[28] = ui->param_current_value_28;
    param_current_value_array[29] = ui->param_current_value_29;
    param_current_value_array[30] = ui->param_current_value_30;

    param_current_value_array[31] = ui->param_current_value_31;
    param_current_value_array[32] = ui->param_current_value_32;
    param_current_value_array[33] = ui->param_current_value_33;
    param_current_value_array[34] = ui->param_current_value_34;
    param_current_value_array[35] = ui->param_current_value_35;
    param_current_value_array[36] = ui->param_current_value_36;

    //checkBox_params[38]

    checkBox_params[0] = ui->checkBox_param_0;
    checkBox_params[1] = ui->checkBox_param_1;
    checkBox_params[2] = ui->checkBox_param_2;
    checkBox_params[3] = ui->checkBox_param_3;
    checkBox_params[4] = ui->checkBox_param_4;

    checkBox_params[13] = ui->checkBox_param_13;
    checkBox_params[14] = ui->checkBox_param_14;
    checkBox_params[15] = ui->checkBox_param_15;
    checkBox_params[16] = ui->checkBox_param_16;
    checkBox_params[17] = ui->checkBox_param_17;

    checkBox_params[21] = ui->checkBox_param_21;
    checkBox_params[22] = ui->checkBox_param_22;
    checkBox_params[23] = ui->checkBox_param_23;
    checkBox_params[24] = ui->checkBox_param_24;
    checkBox_params[25] = ui->checkBox_param_25;
    checkBox_params[26] = ui->checkBox_param_26;
    checkBox_params[27] = ui->checkBox_param_27;
    checkBox_params[28] = ui->checkBox_param_28;
    checkBox_params[29] = ui->checkBox_param_29;

    checkBox_params[30] = ui->checkBox_param_30;
    checkBox_params[31] = ui->checkBox_param_31;
    checkBox_params[32] = ui->checkBox_param_32;
    checkBox_params[33] = ui->checkBox_param_33;
    checkBox_params[34] = ui->checkBox_param_34;
    checkBox_params[35] = ui->checkBox_param_35;
    checkBox_params[36] = ui->checkBox_param_36;

}

void awd::set_mode_items()
{
    ui->comboBox_mode->addItem("Стабилизация скор.по ЭДС");
    ui->comboBox_mode->addItem("Стабилизация скор.по энкодеру");
    ui->comboBox_mode->addItem("Слежение за внешним сигналом");
    ui->comboBox_mode->addItem("Ограничение момента");
}

void awd::set_mode_connections()
{
    connect(ui->SkipLim, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->LimDrop, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->StopDrop, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->IntrfEN, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->IntrfVal, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->IntrfDir, &QCheckBox::stateChanged, this, &awd::change_state);
    connect(ui->SrcParam, &QCheckBox::stateChanged, this, &awd::change_state);
}

void awd::mode_read(const QByteArray &data)
{
    //Data1
        if( ( data[4] & (char)0x40 ) == (char)0x40 ) ui->SkipLim->setChecked(1);
        else ui->SkipLim->setChecked(0);

        if( ( data[4] & (char)0x20 ) == (char)0x20 ) ui->LimDrop->setChecked(1);
        else ui->LimDrop->setChecked(0);

        if( ( data[4] & (char)0x16 ) == (char)0x16 ) ui->StopDrop->setChecked(1);
        else ui->StopDrop->setChecked(0);

        if( ( data[4] & (char)0x08 ) == (char)0x08 ) ui->IntrfEN->setChecked(1);
        else ui->IntrfEN->setChecked(0);

        if( ( data[4] & (char)0x04 ) == (char)0x04 ) ui->IntrfVal->setChecked(1);
        else ui->IntrfVal->setChecked(0);

        if( ( data[4] & (char)0x02 ) == (char)0x02 ) ui->IntrfDir->setChecked(1);
        else ui->IntrfDir->setChecked(0);

        if( ( data[4] & (char)0x01 ) == (char)0x01 ) ui->SrcParam->setChecked(1);
        else ui->SrcParam->setChecked(0);
    //Data0
        if( ( data[5] & (char)0x00 ) == (char)0x00 ) ui->comboBox_mode->setCurrentText("Стабилизация скор.по ЭДС");
        if( ( data[5] & (char)0x01 ) == (char)0x01 ) ui->comboBox_mode->setCurrentText("Стабилизация скор.по энкодеру");
        if( ( data[5] & (char)0x02 ) == (char)0x02 ) ui->comboBox_mode->setCurrentText("Слежение за внешним сигналом");
        if( ( data[5] & (char)0x03 ) == (char)0x03 ) ui->comboBox_mode->setCurrentText("Ограничение момента");

}

void awd::status_no_edit()
{
    ui->StLimFrw_label->setStyleSheet("color: rgb(192,192,192)");
    ui->StLimRev_label->setStyleSheet("color: rgb(192,192,192)");
    ui->StinFrw_label->setStyleSheet("color: rgb(192,192,192)");
    ui->StinRev_label->setStyleSheet("color: rgb(192,192,192)");
    ui->StMotAct_label->setStyleSheet("color: rgb(192,192,192)");
    ui->StDirFrwRev_label->setStyleSheet("color: rgb(192,192,192)");
    ui->StMaxPWM_label->setStyleSheet("color: rgb(192,192,192)");
    ui->StOverCur_label->setStyleSheet("color: rgb(192,192,192)");
}

void awd::status_read(const QByteArray &data)
{

    if( ( data[6] & (char)0x01 ) == (char)0x01 ) ui->StLimFrw_label->setStyleSheet("color: rgb(0,128,0)");
    else ui->StLimFrw_label->setStyleSheet("color: rgb(192,192,192)");

    if( ( data[6] & (char)0x02 ) == (char)0x02 ) ui->StLimRev_label->setStyleSheet("color: rgb(0,128,0)");
    else ui->StLimRev_label->setStyleSheet("color: rgb(192,192,192)");

    if( ( data[6] & (char)0x04 ) == (char)0x04 ) ui->StinFrw_label->setStyleSheet("color: rgb(0,128,0)");
    else ui->StinFrw_label->setStyleSheet("color: rgb(192,192,192)");

    if( ( data[6] & (char)0x08 ) == (char)0x08 ) ui->StinRev_label->setStyleSheet("color: rgb(0,128,0)");
    else ui->StinRev_label->setStyleSheet("color: rgb(192,192,192)");

    if( ( data[6] & (char)0x10 ) == (char)0x10 ) ui->StMotAct_label->setStyleSheet("color: rgb(0,128,0)");
    else ui->StMotAct_label->setStyleSheet("color: rgb(192,192,192)");

    if( ( data[6] & (char)0x20 ) == (char)0x20 ) ui->StDirFrwRev_label->setStyleSheet("color: rgb(0,128,0)");
    else ui->StDirFrwRev_label->setStyleSheet("color: rgb(192,192,192)");

    if( ( data[6] & (char)0x40 ) == (char)0x40 ) ui->StMaxPWM_label->setStyleSheet("color: rgb(0,128,0)");
    else ui->StMaxPWM_label->setStyleSheet("color: rgb(192,192,192)");

    if( ( data[6] & (char)0x80 ) == (char)0x80 ) ui->StOverCur_label->setStyleSheet("color: rgb(0,128,0)");
    else ui->StOverCur_label->setStyleSheet("color: rgb(192,192,192)");

}

void awd::real_plot( const QByteArray &data )
{


    double key = time.elapsed() / 1000.0;


    double lastPointKey = 0;

        if(key - lastPointKey > 0.002)
        {
               if( data[2] == (char)0x00 )
              {
                  qavx1_x.append(key);
                  qavx1_y.append(setReadDataValue(data));
                  qDebug() << "Avx1: " << qavx1_x << ":" << qavx1_y;
                  ui->plot->graph(1)->setData(qavx1_x, qavx1_y);

                  values[0] = setReadDataValue(data);
              }

               if( data[2] == (char)0x01 )
               {
                   qavx2_x.append(key);
                   qavx2_y.append(setReadDataValue(data));
                   qDebug() << "Avx2: " << qavx2_x << " : " << qavx2_y;
                   ui->plot->graph(2)->setData(qavx2_x, qavx2_y);

                   values[1] = setReadDataValue(data);
               }

               if( data[2] == (char)0x05 )
               {
                   qv_x.append(key);
                   qv_y.append(setReadDataValue(data));
                   qDebug() << "speed: " << qv_x << ":" << qv_y;
                   ui->plot->graph(0)->setData(qv_x, qv_y);

                   values[2] = setReadDataValue(data);
               }




               if ( ui->speed_checkBox->isChecked() && !( ui->A_vx_1_checkBox->isChecked() ) && !( ui->A_vx_2_checkBox->isChecked() ) )
               { 
                   ui->plot->xAxis->setRange(qv_x.first() - 2, qv_x.last() + 2);
                   ui->plot->yAxis->setRange(*std::min_element(qv_y.begin(), qv_y.end()) - 2,
                                             *std::max_element(qv_y.begin(), qv_y.end()) + 2);
               }
               else if ( !ui->speed_checkBox->isChecked() && ui->A_vx_1_checkBox->isChecked() && !( ui->A_vx_2_checkBox->isChecked() ) )
               {
                   ui->plot->xAxis->setRange(qavx1_x.first() - 2, qavx1_x.last() + 2);
                   ui->plot->yAxis->setRange(*std::min_element(qavx1_y.begin(), qavx1_y.end()) - 2,
                                             *std::max_element(qavx1_y.begin(), qavx1_y.end()) + 2);
               }
               else if ( !ui->speed_checkBox->isChecked() && !ui->A_vx_1_checkBox->isChecked() && ui->A_vx_2_checkBox->isChecked() )
               {
                   ui->plot->xAxis->setRange(qavx2_x.first() - 2, qavx2_x.last() + 2);
                   ui->plot->yAxis->setRange(*std::min_element(qavx2_y.begin(), qavx2_y.end()) - 2,
                                             *std::max_element(qavx2_y.begin(), qavx2_y.end()) + 2);
               }
               else if( ui->speed_checkBox->isChecked() && ui->A_vx_1_checkBox->isChecked() && !( ui->A_vx_2_checkBox->isChecked() )
                        && !qv_x.empty() && !qv_y.empty() && !qavx1_x.empty() && !qavx1_y.empty() )
               {
                   ui->plot->xAxis->setRange( (qv_x.first() < qavx1_x.first() ? qv_x.first() : qavx1_x.first()) - 2, (qv_x.last() > qavx1_x.last() ? qv_x.last() : qavx1_x.last()) + 2);
                   ui->plot->yAxis->setRange( (*std::min_element(qv_y.begin(), qv_y.end()) < *std::min_element(qavx1_y.begin(), qavx1_y.end()) ?
                                             *std::min_element(qv_y.begin(), qv_y.end()) : *std::min_element(qavx1_y.begin(), qavx1_y.end()) ) - 2,

                                             (*std::max_element(qv_y.begin(), qv_y.end()) > *std::max_element(qavx1_y.begin(), qavx1_y.end()) ?
                                             *std::max_element(qv_y.begin(), qv_y.end()) : *std::max_element(qavx1_y.begin(), qavx1_y.end()) ) + 2);

               }
///
               else if( ui->speed_checkBox->isChecked() && !ui->A_vx_1_checkBox->isChecked() && ui->A_vx_2_checkBox->isChecked()
                        && !qv_x.empty() && !qv_y.empty() && !qavx2_x.empty() && !qavx2_y.empty() )
               {
                   ui->plot->xAxis->setRange( ( qv_x.first() < qavx2_x.first() ? qv_x.first() : qavx2_x.first() ) - 2, ( qv_x.last() > qavx2_x.last() ? qv_x.last() : qavx2_x.last() ) + 2);
                   ui->plot->yAxis->setRange( ( *std::min_element(qv_y.begin(), qv_y.end()) < *std::min_element(qavx2_y.begin(), qavx2_y.end()) ?
                                             *std::min_element(qv_y.begin(), qv_y.end()) : *std::min_element(qavx2_y.begin(), qavx2_y.end()) ) - 2,

                                             (*std::max_element(qv_y.begin(), qv_y.end()) > *std::max_element(qavx2_y.begin(), qavx2_y.end()) ?
                                             *std::max_element(qv_y.begin(), qv_y.end()) : *std::max_element(qavx2_y.begin(), qavx2_y.end()) ) + 2);

               }
///
               else if( !ui->speed_checkBox->isChecked() && ui->A_vx_1_checkBox->isChecked() && ui->A_vx_2_checkBox->isChecked()
                        && !qavx1_x.empty() && !qavx1_y.empty() && !qavx2_x.empty() && !qavx2_y.empty() )
               {
                   ui->plot->xAxis->setRange( ( qavx1_x.first() < qavx2_x.first() ? qavx1_x.first() : qavx2_x.first()) - 2, ( qavx1_x.last() > qavx2_x.last() ? qavx1_x.last() : qavx2_x.last()) + 2);
                   ui->plot->yAxis->setRange((*std::min_element(qavx1_y.begin(), qavx1_y.end()) < *std::min_element(qavx2_y.begin(), qavx2_y.end()) ?
                                             *std::min_element(qavx1_y.begin(), qavx1_y.end()) : *std::min_element(qavx2_y.begin(), qavx2_y.end()) ) - 2,

                                             (*std::max_element(qavx1_y.begin(), qavx1_y.end()) > *std::max_element(qavx2_y.begin(), qavx2_y.end()) ?
                                             *std::max_element(qavx1_y.begin(), qavx1_y.end()) : *std::max_element(qavx2_y.begin(), qavx2_y.end())) + 2);

               }
////
               else if( ui->speed_checkBox->isChecked() && ui->A_vx_1_checkBox->isChecked() && ui->A_vx_2_checkBox->isChecked()
                       && !qv_x.empty() && !qv_y.empty() && !qavx1_x.empty() && !qavx1_y.empty() && !qavx2_x.empty() && !qavx2_y.empty() )
               {
                   ui->plot->xAxis->setRange(std::min(std::min(qv_x.first(), qavx1_x.first()), qavx2_x.first()) - 2,
                                             std::max(std::max(qv_x.last(), qavx1_x.last()), qavx2_x.last()) + 2);

                   ui->plot->yAxis->setRange(std::min(std::min(*std::min_element(qv_y.begin(), qv_y.end()),
                                                      *std::min_element(qavx2_y.begin(), qavx2_y.end())),
                                                      *std::min_element(qavx2_y.begin(), qavx2_y.end()) ) - 2,
                                             std::max(std::max( *std::max_element(qv_y.begin(), qv_y.end()),
                                                      *std::max_element(qavx1_y.begin(), qavx1_y.end()) ),
                                                      *std::max_element(qavx2_y.begin(), qavx2_y.end()) ) + 2 );

               }
////

            ui->plot->replot();
            ui->plot->update();
            lastPointKey = key;
        }

        ui->plot->replot();
}

void awd::slot_for_new_point()
{
    if(chek)
    {

    if( ui->A_vx_1_checkBox->isChecked() )
    {
        command[1] = 0x3c;
        command[2] = 0x00;//00
        command[3] = 0x00;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0x00;
        command[7] = checkSumm(command);

        writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));

    }

     if( ui->A_vx_2_checkBox->isChecked() )
    {
        command[1] = 0x3c;
        command[2] = 0x01;//01
        command[3] = 0x00;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0x00;
        command[7] = checkSumm(command);

        writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));

    }

    if( ui->speed_checkBox->isChecked() )
    {
        command[1] = 0x3c;
        command[2] = 0x05;//05
        command[3] = 0x00;
        command[4] = 0x00;
        command[5] = 0x00;
        command[6] = 0x00;
        command[7] = checkSumm(command);

        writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));

    }


    if( qv_x.empty() && qavx1_x.empty() && qavx2_x.empty() ) // 000
    {
        values = {0,0,0};
        graph_value[0] = values;
    }

    else if ( !qv_x.empty() && qavx1_x.empty() && qavx2_x.empty() ) // 001
    {
        graph_value[qv_x.last()] = values;
    }
    else if ( qv_x.empty() && !qavx1_x.empty() && qavx2_x.empty() ) // 010
    {
        graph_value[qavx1_x.last()] = values;
    }
    else if ( qv_x.empty() && qavx1_x.empty() && !qavx2_x.empty() ) // 100
    {
        graph_value[qavx2_x.last()] = values;
    }
    else if( !qv_x.empty() && qavx1_x.empty() && !qavx2_x.empty() ) // 011
    {
        graph_value[std::max( qv_x.last(), qavx2_x.last() )] = values;
    }
///
    else if( !qv_x.empty() && !qavx1_x.empty() && qavx2_x.empty() )
    {
        graph_value[std::max( qv_x.last(), qavx1_x.last() )] = values;
    }
///
    else if( qv_x.empty() && !qavx1_x.empty() && !qavx2_x.empty() )
    {
        graph_value[std::max( qavx2_x.last(), qavx1_x.last() )] = values;
    }
////
    else if( !qv_x.empty() && !qavx1_x.empty() && !qavx2_x.empty() )
    {
        graph_value[std::max(std::max( qv_x.last(), qavx1_x.last() ), qavx2_x.last())] = values;
    }


        values = {0,0,0};
    }
}

void awd::chart_update_period(const int &value)
{
    timer->start(value);
}

void awd::slotMouseMove(QMouseEvent *event)
{

/*
    // Определение координаты X на графике, где был произведён клик мышью
    double coordX = ui->plot->xAxis->pixelToCoord(event->pos().x());

    // По координате X клика мыши определим ближайшие координаты для трассировщика
    tracer->setGraphKey(coordX);

    // Координаты точкек графика скорости в label
    ui->tracer_value->setText("x: " + QString::number(tracer->position->key()) +
                              " y: " + QString::number(tracer->position->value()));

    ui->plot->replot(); // Перерисовываем содержимое полотна графика
*/

    QCustomPlot* customPlot = qobject_cast<QCustomPlot*>(sender());
    double x = customPlot->xAxis->pixelToCoord(event->pos().x());
    double y = customPlot->yAxis->pixelToCoord(event->pos().y());


    hLine->setPen(Qt::DashDotLine);
    hLine->start->setCoords(-QCPRange::maxRange, y);
    hLine->end->setCoords(QCPRange::maxRange, y);

    vLine->setPen(Qt::DashDotLine);
    vLine->start->setCoords(x, -QCPRange::maxRange);
    vLine->end->setCoords(x, QCPRange::maxRange);

    cursorText->setText(QString("(%1, %2)").arg(x).arg(y));
    cursorText->position->setCoords(QPointF(x, y));
    cursorText->setFont(QFont(font().family(), 11));

    QPointF pp = cursorText->position->pixelPosition() + QPointF(70.0, -15.0);
    cursorText->position->setPixelPosition(pp);

    customPlot->replot();
}

void awd::on_write_all_by_default_clicked()
{
     qDebug() << "adress write all: " << command[0] ;

    for (int i = 0; i <= 36; i++ ) {
         command_formation(QString::number(by_default[i]),i);
        if(i == 5 || i == 6 || i==7 || i == 8 ||
           i == 9 || i == 10 || i== 11 || i == 12 ||
           i == 18 || i == 19 || i== 20 ){ }
        else
        param_current_value_array[i]->setText(QString::number(by_default[i]));
    }
}

void awd::on_read_all_clicked()
{
    for(int i = 0; i <= 36; i++ ){
        command_formation(i);
        if(i == 5 || i == 6 || i==7 || i == 8 ||
           i == 9 || i == 10 || i== 11 || i == 12 ||
           i == 18 || i == 19 || i== 20 ){ }
        else
        param_current_value_array[i]->setText(QString::number(current_param_value[i]));
    }
}

void awd::on_save_to_file_clicked()
{

    //QFile file(".//params.txt");
    QFile file(QFileDialog::getSaveFileName(this,  "Save file", "С:://", tr("Text files (*.txt)")));

    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, "title", "file not open");
    }

    QTextStream out(&file);
    //out << "Name\t" << "Corrent value\t\n";// создание заголовка для файла
    // ввод значений
    for (int i = 0; i < 37; i++ )
    {
        out << param_name[i] << ":\t"  << current_param_value[i] << "\t\n";
    }

    file.flush();
    file.close();
}

void awd::on_load_from_file_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                               "My title",
                               "This operation will owerwrite all parameters.\n"
                               "Are you sure you want to perform this this operation?",
                                QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes)
{
    QString filter = "AllFile (*.*) ;; Text File (*.txt)";
    QString file_name = QFileDialog::getOpenFileName(this,  "opena file", "С:://", filter);
    QFile file(file_name);

    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Error", "File not open");
    }

    QTextStream in(&file);

    //QString text /*= in.readAll()*/;

    while (!in.atEnd())
    {
        for ( int i = 0; i < 38 ; i++ )
        {
           QString text = in.readLine();
           if (!text.isEmpty())
           {
              QStringList line = text.split('\t');
              if(i != 0)
              {
                 //param_name[i] = line[0];
                 current_param_value[i] = QString(line[1]).toInt();
              }
           }
        }
    }
    file.close();
}

}

void awd::on_write_selected_clicked()
{

    if(ui->checkBox_param_0->isChecked())
    {
        command_formation(ui->param_new_value_0->text(),0);
        ui->param_current_value_0->setText(ui->param_new_value_0->text());
        current_param_value[0] = (ui->param_new_value_0->text()).toInt();
    }
    if(ui->checkBox_param_1->isChecked())
    {
        command_formation(ui->param_new_value_1->text(),1);
        ui->param_current_value_1->setText(ui->param_new_value_1->text());
        current_param_value[1] = (ui->param_new_value_1->text()).toInt();
    }
    if(ui->checkBox_param_2->isChecked())
    {
        command_formation(ui->param_new_value_2->text(),2);
        ui->param_current_value_2->setText(ui->param_new_value_2->text());
        current_param_value[2] = (ui->param_new_value_2->text()).toInt();
    }
    if(ui->checkBox_param_3->isChecked())
    {
        command_formation(ui->param_new_value_3->text(),3);
        ui->param_current_value_3->setText(ui->param_new_value_3->text());
        current_param_value[3] = (ui->param_new_value_3->text()).toInt();
    }
    if(ui->checkBox_param_4->isChecked())
    {
        command_formation(ui->param_new_value_4->text(),4);
        ui->param_current_value_4->setText(ui->param_new_value_4->text());
        current_param_value[4] = (ui->param_new_value_4->text()).toInt();
    }
    if(ui->checkBox_param_13->isChecked())
    {
        command_formation(ui->param_new_value_13->text(),13);
        ui->param_current_value_13->setText(ui->param_new_value_13->text());
        current_param_value[13] = (ui->param_new_value_13->text()).toInt();
    }
    if(ui->checkBox_param_14->isChecked())
    {
        command_formation(ui->param_new_value_14->text(),14);
        ui->param_current_value_14->setText(ui->param_new_value_14->text());
        current_param_value[14] = (ui->param_new_value_14->text()).toInt();
    }
    if(ui->checkBox_param_15->isChecked())
    {
        command_formation(ui->param_new_value_15->text(),15);
        ui->param_current_value_15->setText(ui->param_new_value_15->text());
        current_param_value[15] = (ui->param_new_value_15->text()).toInt();
    }
    if(ui->checkBox_param_16->isChecked())
    {
        command_formation(ui->param_new_value_16->text(),16);
        ui->param_current_value_16->setText(ui->param_new_value_16->text());
        current_param_value[16] = (ui->param_new_value_16->text()).toInt();
    }
    if(ui->checkBox_param_17->isChecked())
    {
        command_formation(ui->param_new_value_17->text(),17);
        ui->param_current_value_17->setText(ui->param_new_value_17->text());
        current_param_value[17] = (ui->param_new_value_17->text()).toInt();
    }
    if(ui->checkBox_param_21->isChecked())
    {
        command_formation(ui->param_new_value_21->text(),21);
        ui->param_current_value_21->setText(ui->param_new_value_21->text());
        current_param_value[21] = (ui->param_new_value_21->text()).toInt();
    }
    if(ui->checkBox_param_22->isChecked())
    {
        command_formation(ui->param_new_value_22->text(),22);
        ui->param_current_value_22->setText(ui->param_new_value_22->text());
        current_param_value[22] = (ui->param_new_value_22->text()).toInt();
    }
    if(ui->checkBox_param_23->isChecked())
    {
        command_formation(ui->param_new_value_23->text(),23);
        ui->param_current_value_23->setText(ui->param_new_value_23->text());
        current_param_value[23] = (ui->param_new_value_23->text()).toInt();
    }
    if(ui->checkBox_param_24->isChecked())
    {
        command_formation(ui->param_new_value_24->text(),24);
        ui->param_current_value_24->setText(ui->param_new_value_24->text());
        current_param_value[24] = (ui->param_new_value_24->text()).toInt();
    }
    if(ui->checkBox_param_25->isChecked())
    {
        command_formation(ui->param_new_value_25->text(),25);
        ui->param_current_value_25->setText(ui->param_new_value_25->text());
        current_param_value[25] = (ui->param_new_value_25->text()).toInt();
    }
    if(ui->checkBox_param_26->isChecked())
    {
        command_formation(ui->param_new_value_26->currentText(),26);
        ui->param_current_value_26->setText(ui->param_new_value_26->currentText());
        current_param_value[26] = (ui->param_new_value_26->currentText()).toInt();
    }
    if(ui->checkBox_param_27->isChecked())
    {
        command_formation(ui->param_new_value_27->text(),27);
        ui->param_current_value_27->setText(ui->param_new_value_27->text());
        current_param_value[27] = (ui->param_new_value_27->text()).toInt();
    }
    if(ui->checkBox_param_28->isChecked())
    {
        command_formation(ui->param_new_value_28->text(),28);
        ui->param_current_value_28->setText(ui->param_new_value_28->text());
        current_param_value[28] = (ui->param_new_value_28->text()).toInt();
    }
    if(ui->checkBox_param_29->isChecked())
    {
        command_formation(ui->param_new_value_29->text(),29);
        ui->param_current_value_29->setText(ui->param_new_value_29->text());
        current_param_value[29] = (ui->param_new_value_29->text()).toInt();
    }
    if(ui->checkBox_param_30->isChecked())
    {
        command_formation(ui->param_new_value_30->text(),30);
        ui->param_current_value_30->setText(ui->param_new_value_30->text());
        current_param_value[30] = (ui->param_new_value_30->text()).toInt();
    }
    if(ui->checkBox_param_31->isChecked())
    {
        command_formation(ui->param_new_value_31->text(),31);
        ui->param_current_value_31->setText(ui->param_new_value_31->text());
        current_param_value[31] = (ui->param_new_value_31->text()).toInt();
    }
    if(ui->checkBox_param_32->isChecked())
    {
        command_formation(ui->param_new_value_32->text(),32);
        ui->param_current_value_32->setText(ui->param_new_value_32->text());
        current_param_value[32] = (ui->param_new_value_32->text()).toInt();
    }
    if(ui->checkBox_param_33->isChecked())
    {
        command_formation(ui->param_new_value_33->text(),33);
        ui->param_current_value_33->setText(ui->param_new_value_33->text());
        current_param_value[33] = (ui->param_new_value_33->text()).toInt();
    }
    if(ui->checkBox_param_34->isChecked())
    {
        command_formation(ui->param_new_value_34->text(),34);
        ui->param_current_value_34->setText(ui->param_new_value_34->text());
        current_param_value[34] = (ui->param_new_value_34->text()).toInt();
    }
    if(ui->checkBox_param_35->isChecked())
    {
        command_formation(ui->param_new_value_35->text(),35);
        ui->param_current_value_35->setText(ui->param_new_value_35->text());
        current_param_value[35] = (ui->param_new_value_35->text()).toInt();
    }
    if(ui->checkBox_param_36->isChecked())
    {
        command_formation(ui->param_new_value_36->text(),36);
        ui->param_current_value_36->setText(ui->param_new_value_36->text());
        current_param_value[36] = (ui->param_new_value_36->text()).toInt();
    }
}

void awd::change_state()
{
    //SkipLim
    if(ui->SkipLim->isChecked()) ui->SkipLim->setText("Не использовать сигналы от концевых выключателей");
    else ui->SkipLim->setText("Использовать сигналы от концевых выключателей");

    //LimDrop
    if(ui->LimDrop->isChecked()) ui->LimDrop->setText("Не удерживать двигатель при срабатывании концевого выключателя");
    else ui->LimDrop->setText("Удерживать двигатель при срабатывании концевого выключателя");

    //StopDrop
    if(ui->StopDrop->isChecked()) ui->StopDrop->setText("Не Удерживать двигатель при остановке вращения");
    else ui->StopDrop->setText("Удерживать двигатель при остановке вращения");

    //IntrfEN
    if(ui->IntrfEN->isChecked()) ui->IntrfEN->setText("Управлять разрешением вращения через интерфейс RS485");
    else ui->IntrfEN->setText("Управлять разрешением вращения с помощью внешнего сигнала");

    //IntrfVal
    if(ui->IntrfVal->isChecked()) ui->IntrfVal->setText("Управлять скоростью вращения через интерфейс RS485");
    else ui->IntrfVal->setText("Управлять сокростью с помощью внешних сигналов");

    //IntrfDir
    if(ui->IntrfDir->isChecked()) ui->IntrfDir->setText("Управлять направлением через интерфейс RS485");
    else ui->IntrfDir->setText("Управлять направлением с помощью внешних сигналов");

    //SrcParam
    if(ui->SrcParam->isChecked()) ui->SrcParam->setText("Использовать аналоговый выход №2");
    else ui->SrcParam->setText("Использовать параметр №31");

}

void awd::on_read_button_regime_clicked()
{
    command[1] = 0x87;
    command[2] = 0x1c;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_write_button_regime_clicked()
{
    //остановка режима перед изменением режима
    command[1] = 0x4b;
    command[2] = 0x0a;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));

    command[1] = 0x78;
    command[2] = 0x1c;
    command[3] = 0x00;

//Data1
    if(ui->SkipLim->isChecked()) command[4] |= 64;
    if(ui->LimDrop->isChecked()) command[4] |= 32;
    if(ui->StopDrop->isChecked()) command[4] |= 16;
    if(ui->IntrfEN->isChecked()) command[4] |= 8;
    if(ui->IntrfVal->isChecked()) command[4] |= 4;
    if(ui->IntrfDir->isChecked()) command[4] |= 2;
    if(ui->SrcParam->isChecked()) command[4] |= 1;
//Data0
    if(ui->comboBox_mode->currentText() == "Стабилизация скор.по ЭДС") command[5] |= 0;
    if(ui->comboBox_mode->currentText() == "Стабилизация скор.по энкодеру") command[5] |= 1;
    if(ui->comboBox_mode->currentText() == "Слежение за внешним сигналом") command[5] |= 2;
    if(ui->comboBox_mode->currentText() == "Ограничение момента") command[5] |= 3;

    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
    ///////////////
//обнуление для следующего запуска
    command[4] = 0;
    command[5] = 0;
}

void awd::on_stop_mode_button_clicked()
{
    command[1] = 0x4b;
    command[2] = 0x0a;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_start_tracking_clicked()
{
    command[1] = 0x4b;
    command[2] = 0x0b;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}


void awd::on_speed_spinBox_editingFinished()
{
    command[1] = 0x4b;
    command[2] = 0x08;
    command[3] = 0x00;
    command[4] = (ui->speed_spinBox->value() >> 8) & 0xFF;
    command[5] = ui->speed_spinBox->value() & 0xFF;
    command[6] = 0x00;
    command[7] = checkSumm(command);//вычисление контрольной суммы

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));

    ui->speed_horizontalSlider->setValue(ui->speed_spinBox->value());
}

void awd::on_stop_button_clicked()
{   
    command[1] = 0x4b;
    command[2] = 0x08;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
    ui->speed_spinBox->setValue(0);
    ui->speed_horizontalSlider->setValue(0);

    command[7] = 0x00;
}

void awd::on_Kp_spinBox_valueChanged(const QString &arg1)
{
    ui->param_current_value_15->setText(arg1);
}

void awd::on_Ki_spinBox_valueChanged(const QString &arg1)
{
    ui->param_current_value_16->setText(arg1);
}

void awd::on_Kd_spinBox_valueChanged(const QString &arg1)
{
    ui->param_current_value_17->setText(arg1);
}


void awd::plot_settings()
{
    ui->plot->setInteraction(QCP::iRangeDrag, true);// взаимодействие удаления/приближения графика
    ui->plot->setInteraction(QCP::iRangeZoom, true);// взвимодействие перетаскивания графика

    ui->plot->addGraph();
    ui->plot->graph(0)->setPen(QPen(QColor(0, 255, 127)));

    ui->plot->addGraph();
    ui->plot->graph(1)->setPen(QPen(QColor(255, 0, 0)));

    ui->plot->addGraph();
    ui->plot->graph(2)->setPen(QPen(QColor(100, 0, 170)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s");
    ui->plot->xAxis->setTicker(timeTicker);
    ui->plot->axisRect()->setupFullAxesBox();

}

void awd::on_btn_clear_clicked()
{
    //clearData

    if( !qv_x.empty() && !qv_y.empty() )
    {
        qv_x.clear();
        qv_y.clear();
    }


    if( !qavx1_x.empty() && !qavx1_y.empty() )
    {
        qavx1_x.clear();
        qavx1_y.clear();
    }

    if( !qavx2_x.empty() && !qavx2_y.empty() )
    {
        qavx2_x.clear();
        qavx2_y.clear();
    }

    if(!graph_value.empty())
    {
        graph_value.clear();
    }

    // plot
    ui->plot->graph(0)->setData(qv_x, qv_y);
    ui->plot->graph(1)->setData(qavx1_x, qavx1_y);
    ui->plot->graph(2)->setData(qavx2_x, qavx2_y);

    ui->plot->replot();
    ui->plot->update();

    time.start(); // старт времени графика

}

void awd::writeData(const QByteArray &data)
{
    if(serial->isOpen() == true){
        qDebug() << "write port : " << serial->portName();
        serial->write(data);
        serial->waitForBytesWritten();
        if (serial->isWritable())
            qDebug() << "write: " << data;
        ui->write_label->setText(data.toHex( ' ' ));

        readData();
    }
    else
    {
        qDebug() << "not open";
        ui->read_label->setText("Ошибка ввода");
    }
    command[7] = 0x00;// сброс контрольной суммы после записи
}

char16_t awd::setReadDataValue(const QByteArray &data)
{
    return  (( ( data[4] << 8  ) | ( data[5] & 0xff ) ));
}

void awd::readData()
{
    QByteArray data;
    if(serial->isReadable())
    {
       while ( serial->waitForReadyRead(reply_time) )
       {
           // результат чтения накапливается в переменную data
           data.append(serial->readAll());

           if( data.size() == 8 ) break;
       }
       qDebug() << "read: " << data;

//Эхо
    if( data[1] == (char)0xf0 && data[2] == (char)0x41 &&
        data[3] == (char)0x57 && data[4] == (char)0x44 )
    {
      exo = true;
    }

//Статус
    //if(exo)
//{
    if(ui->status_checkBox->isChecked())
    {
        status_read(data);
    }

     else {
        ui->StLimFrw_label->setStyleSheet("color: rgb(192,192,192)");
        ui->StLimRev_label->setStyleSheet("color: rgb(192,192,192)");
        ui->StinFrw_label->setStyleSheet("color: rgb(192,192,192)");
        ui->StinRev_label->setStyleSheet("color: rgb(192,192,192)");
        ui->StMotAct_label->setStyleSheet("color: rgb(192,192,192)");
        ui->StDirFrwRev_label->setStyleSheet("color: rgb(192,192,192)");
        ui->StMaxPWM_label->setStyleSheet("color: rgb(192,192,192)");
        ui->StOverCur_label->setStyleSheet("color: rgb(192,192,192)");
    }
//}
//Режим
    if( data[1] == (char)0x87 && data[2] == (char)0x1c )
    {
        mode_read(data);
    }
//График
    if(data[1] == (char)0x3c)
    {
        real_plot(data);
    }

// Параметры
       if(data[1] == (char)0x87 && data[2] == (char)0x00)
       {
           ui->param_current_value_0->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_0->setValue(setReadDataValue(data));
           current_param_value[0] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x01)
       {
           ui->param_current_value_1->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_1->setValue(setReadDataValue(data));
           current_param_value[1] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x02)
       {
           ui->param_current_value_2->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_2->setValue(setReadDataValue(data));
           current_param_value[2] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x03)
       {
           ui->param_current_value_3->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_3->setValue(setReadDataValue(data));
           current_param_value[3] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x04)
       {
           ui->param_current_value_4->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_4->setValue(setReadDataValue(data));
           current_param_value[4] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x0d)
       {
           ui->param_current_value_13->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_13->setValue(setReadDataValue(data));
           current_param_value[13] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x0e)
       {
           ui->param_current_value_14->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_14->setValue(setReadDataValue(data));
           current_param_value[14] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x0f) // Kp
       {
           ui->param_current_value_15->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_15->setValue(setReadDataValue(data));

           ui->Kp->setValue(setReadDataValue(data));

           current_param_value[15] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x10) // Ki
       {
           ui->param_current_value_16->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_16->setValue(setReadDataValue(data));

           ui->Ki->setValue(setReadDataValue(data));

           current_param_value[16] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x11) // Kd
       {
           ui->param_current_value_17->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_17->setValue(setReadDataValue(data));

           ui->Kd->setValue(setReadDataValue(data));

           current_param_value[17] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x15)
       {
           ui->param_current_value_21->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_21->setValue(setReadDataValue(data));
           current_param_value[21] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x16)
       {
           ui->param_current_value_22->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_22->setValue(setReadDataValue(data));
           current_param_value[22] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x17)
       {
           ui->param_current_value_23->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_23->setValue(setReadDataValue(data));
           current_param_value[23] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x18)
       {
           ui->param_current_value_24->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_24->setValue(setReadDataValue(data));
           current_param_value[24] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x19)
       {
           ui->param_current_value_25->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_25->setValue(setReadDataValue(data));
           current_param_value[25] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1a)
       {
           ui->param_current_value_26->setText(QString::number(setReadDataValue(data)));
           current_param_value[26] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1b)
       {
           ui->param_current_value_27->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_27->setValue(setReadDataValue(data));
           current_param_value[27] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1c)
       {
           ui->param_current_value_28->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_28->setValue(setReadDataValue(data));
           current_param_value[28] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1d)
       {
           ui->param_current_value_29->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_29->setValue(setReadDataValue(data));
           current_param_value[29] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1e)
       {
           ui->param_current_value_30->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_30->setValue(setReadDataValue(data));
           current_param_value[30] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x1f)
       {
           ui->param_current_value_31->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_31->setValue(setReadDataValue(data));
           current_param_value[31] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x20)
       {
           ui->param_current_value_32->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_32->setValue(setReadDataValue(data));
           current_param_value[32] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x21)
       {
           ui->param_current_value_33->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_33->setValue(setReadDataValue(data));
           current_param_value[33] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x22)
       {
           ui->param_current_value_34->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_34->setValue(setReadDataValue(data));
           current_param_value[34] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x23)
       {
           ui->param_current_value_35->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_35->setValue(setReadDataValue(data));
           current_param_value[35] = setReadDataValue(data);
       }
       if(data[1] == (char)0x87 && data[2] == (char)0x24)
       {
           ui->param_current_value_36->setText(QString::number(setReadDataValue(data)));
           ui->param_new_value_36->setValue(setReadDataValue(data));
           current_param_value[36] = setReadDataValue(data);
       }

       ui->read_label->setText(data.toHex( ' ' ));
    } else qDebug() << "no answer";

    command[7] = 0x00;
}

void awd::on_spinBox_period_editingFinished()
{
    chart_update_period(ui->spinBox_period->value());
}


void awd::on_export_button_clicked()
{
    QFile file(QFileDialog::getSaveFileName(this,  "Save file", "С:://", tr("Text files (*.txt)")));

    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        //QMessageBox::warning(this, "title", "file not open");
    }

    QTextStream out(&file);

    out << "Time\t\t" << "ADC_1\t\t" << "ADC_2\t\t" << "Speed\t\n";

    for (const auto& item : graph_value)
    {
        QString str = QString::number(item.first);
        std::replace(str.begin(), str.end(), '.', ',');
        out << str << "\t\t ";

        for (const double& stop : item.second)
        {
            out << stop << "\t\t ";
        }
        out << "\t\n";
    }


    file.flush();
    file.close();

}

void awd::on_address_edit_editingFinished()
{

    command[0] = (ui->address_edit->text()).toInt();

    qDebug() << "adress spin box: " << command[0];

    command[1] = 0xf0;
    command[2] = 0x00;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));

    if(exo)
    {
        ui->tabWidget->setEnabled(1);
        ui->tab_5->setEnabled(1);
        ui->scrollArea_2->setEnabled(1);
        ui->scrollAreaWidgetContents_2->setEnabled(1);
        ui->speed_horizontalSlider->setEnabled(1);
        ui->status_checkBox->setEnabled(1);

        ui->address_edit->setEnabled(0);
    }
    else
    {
        ui->read_label->setText("Ошибка!");
    }
    exo = false;
}


void awd::on_write_pid_clicked()
{
    command[1] = 0x78;
    command[2] = 0x0F;
    command[3] = 0x00;
    command[4] = (ui->Kp->value() >> 8) & 0xFF;
    command[5] = ui->Kp->value() & 0xFF;
    command[6] = 0x00;
    command[7] = checkSumm(command); //вычисление контрольной суммы

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));


    command[1] = 0x78;
    command[2] = 0x10;
    command[3] = 0x00;
    command[4] = (ui->Ki->value() >> 8) & 0xFF;
    command[5] = ui->Ki->value() & 0xFF;
    command[6] = 0x00;
    command[7] = checkSumm(command);//вычисление контрольной суммы

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));


    command[1] = 0x78;
    command[2] = 0x11;
    command[3] = 0x00;
    command[4] = (ui->Kd->value() >> 8) & 0xFF;
    command[5] = ui->Kd->value() & 0xFF;
    command[6] = 0x00;
    command[7] = checkSumm(command);//вычисление контрольной суммы

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}


void awd::on_read_pid_clicked()
{

    command[1] = 0x87;
    command[2] = 0x0F;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));


    command[1] = 0x87;
    command[2] = 0x10;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));


    command[1] = 0x87;
    command[2] = 0x11;
    command[3] = 0x00;
    command[4] = 0x00;
    command[5] = 0x00;
    command[6] = 0x00;
    command[7] = checkSumm(command);

    writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_checkBox_select_all_clicked(bool checked)
{
    if(checked)
    {
        ui->checkBox_param_0->setChecked(1);
        ui->checkBox_param_1->setChecked(1);
        ui->checkBox_param_2->setChecked(1);
        ui->checkBox_param_3->setChecked(1);
        ui->checkBox_param_4->setChecked(1);

        ui->checkBox_param_13->setChecked(1);
        ui->checkBox_param_14->setChecked(1);
        ui->checkBox_param_15->setChecked(1);
        ui->checkBox_param_16->setChecked(1);
        ui->checkBox_param_17->setChecked(1);

        ui->checkBox_param_21->setChecked(1);
        ui->checkBox_param_22->setChecked(1);
        ui->checkBox_param_23->setChecked(1);
        ui->checkBox_param_24->setChecked(1);
        ui->checkBox_param_25->setChecked(1);
        ui->checkBox_param_26->setChecked(1);
        ui->checkBox_param_27->setChecked(1);
        ui->checkBox_param_28->setChecked(1);
        ui->checkBox_param_29->setChecked(1);
        ui->checkBox_param_30->setChecked(1);
        ui->checkBox_param_31->setChecked(1);
        ui->checkBox_param_32->setChecked(1);
        ui->checkBox_param_33->setChecked(1);
        ui->checkBox_param_34->setChecked(1);
        ui->checkBox_param_35->setChecked(1);
        ui->checkBox_param_36->setChecked(1);
    }

    else
    {
        ui->checkBox_param_0->setChecked(0);
        ui->checkBox_param_1->setChecked(0);
        ui->checkBox_param_2->setChecked(0);
        ui->checkBox_param_3->setChecked(0);
        ui->checkBox_param_4->setChecked(0);

        ui->checkBox_param_13->setChecked(0);
        ui->checkBox_param_14->setChecked(0);
        ui->checkBox_param_15->setChecked(0);
        ui->checkBox_param_16->setChecked(0);
        ui->checkBox_param_17->setChecked(0);

        ui->checkBox_param_21->setChecked(0);
        ui->checkBox_param_22->setChecked(0);
        ui->checkBox_param_23->setChecked(0);
        ui->checkBox_param_24->setChecked(0);
        ui->checkBox_param_25->setChecked(0);
        ui->checkBox_param_26->setChecked(0);
        ui->checkBox_param_27->setChecked(0);
        ui->checkBox_param_28->setChecked(0);
        ui->checkBox_param_29->setChecked(0);
        ui->checkBox_param_30->setChecked(0);
        ui->checkBox_param_31->setChecked(0);
        ui->checkBox_param_32->setChecked(0);
        ui->checkBox_param_33->setChecked(0);
        ui->checkBox_param_34->setChecked(0);
        ui->checkBox_param_35->setChecked(0);
        ui->checkBox_param_36->setChecked(0);
    }

}

void awd::speed_SliderValueChanged(int value)
{
        Qt::MouseButtons btns = QApplication::mouseButtons();
        QPoint localMousePos = ui->speed_horizontalSlider->mapFromGlobal(QCursor::pos());
        bool clickOnSlider = (btns & Qt::LeftButton) &&
                             (localMousePos.x() >= 0 && localMousePos.y() >= 0 &&
                              localMousePos.x() < ui->speed_horizontalSlider->size().width() &&
                              localMousePos.y() < ui->speed_horizontalSlider->size().height());
        if (clickOnSlider)
        {

            float posRatio = localMousePos.x() / (float )ui->speed_horizontalSlider->size().width();
            int sliderRange = ui->speed_horizontalSlider->maximum() - ui->speed_horizontalSlider->minimum();
            int sliderPosUnderMouse = ui->speed_horizontalSlider->minimum() + sliderRange * posRatio;
            if (sliderPosUnderMouse != value)
            {
                ui->speed_horizontalSlider->setValue(sliderPosUnderMouse);
                return;
            }
        }
        command[1] = 0x4b;
        command[2] = 0x08;
        command[3] = 0x00;
        command[4] = (value >> 8) & 0xFF;
        command[5] = value & 0xFF;
        command[6] = 0x00;
        command[7] = checkSumm(command);

        ui->speed_spinBox->setValue(value);

        writeData((QByteArray::fromRawData((const char*)command, sizeof (command))));
}

void awd::on_start_session_clicked()
{
    chek = true;
    ui->A_vx_1_checkBox->setEnabled(0);
    ui->A_vx_2_checkBox->setEnabled(0);
    ui->speed_checkBox->setEnabled(0);
}

void awd::on_stop_session_clicked()
{
    chek = false;

    ui->A_vx_1_checkBox->setEnabled(1);
    ui->A_vx_2_checkBox->setEnabled(1);
    ui->speed_checkBox->setEnabled(1);
}
