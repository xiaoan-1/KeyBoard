#include "keyboard.h"
#include "ui_keyboard.h"

#include <QGraphicsDropShadowEffect>
#include <QDebug>

KeyBoard::KeyBoard(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::KeyBoard)
{
    ui->setupUi(this);

    // 默认隐藏
    this->close();

    // 初始化
    this->init();

    // 初始化按键事件映射
    this->initKeyMap();

    // 绑定按钮键盘事件
    this->bindKeyEvent();
}

KeyBoard::~KeyBoard()
{
    delete ui;
}

/**
 * @author  XiaoAn
 * @brief   单例模式
 * @date    2024-09-06
 */
KeyBoard* KeyBoard::keyBoard = nullptr;
KeyBoard* KeyBoard::instance(QWidget *parent)
{
    if(keyBoard == nullptr){
        keyBoard = new KeyBoard(parent);
    }
    return keyBoard;
}


/**
 * @author  XiaoAn
 * @brief   初始化控件
 * @date    2024-09-06
 */
void KeyBoard::init()
{
    //实例阴影shadow
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    //设置阴影距离
    shadow->setOffset(0, 0);
    //设置阴影颜色
    shadow->setColor(QColor(0, 0, 0));
    //设置阴影圆角
    shadow->setBlurRadius(20);
    //给垂直布局器设置边距
    this->setContentsMargins(10, 10, 10, 10);
    //给嵌套QWidget设置阴影
    this->setGraphicsEffect(shadow);

    // 拖拽控件
    ui->widget_move->setMinimumWidth(this->width() * 0.8);
    ui->widget_move->installEventFilter(this);

    // 父窗口的所有控件安装事件过滤器
    this->parentWidget()->installEventFilter(this);
    QList<QWidget*> widgets = this->parent()->findChildren<QWidget*>();
    // 给每个输入框添加过滤器
    for (QWidget* w : widgets) {
        // 排除自身
        if(w != this && !this->isAncestorOf(w)){
            w->installEventFilter(this);
        }
    }


    // 缓存按键按钮
    QList<QPushButton*> buttons = this->parent()->findChildren<QPushButton *>();
    foreach (QPushButton *button, buttons) {
        // 默认单字符类 按键
        if(button->text().size() == 1){

            // 单字符类按键用于符号切换
            this->switchButton.append(button);

            // 缓存按键按钮
            this->buttonList.append(qMakePair(button, button->text()));
            // 设置默认的按键属性值
            button->setProperty("key", button->text());
        }
    }

    // 空格
    this->buttonList.append(qMakePair(ui->pushButton_space, QString(" ")));
    ui->pushButton_space->setProperty("key", " ");

    // 删除
    this->buttonList.append(qMakePair(ui->pushButton_delete, QString("Backspace")));
    ui->pushButton_delete->setProperty("key", "Backspace");
}

/**
 * @author  XiaoAn
 * @brief   初始化按键字符的映射
 * @date    2024-09-07
 */
void KeyBoard::initKeyMap()
{
    // ---------------------数字类按键------------------------
    for (int i = 0; i < 10; ++i) {
        this->numberKeyMap.insert(QString('0' + i), static_cast<Qt::Key>(Qt::Key_0 + i));
    }
    // ---------------------字母类按键------------------------
    for (int i = 0; i < 26; ++i) {
        this->letterKeyMap.insert(QString('A' + i), static_cast<Qt::Key>(Qt::Key_A + i));
    }

    // ---------------------字符类按键------------------------
    // 字符按键1
    for(int i = 0; i < 15; ++i){
        this->symbolKeyMap.insert(QString('!' + i), static_cast<Qt::Key>(Qt::Key_Exclam + i));
    }

    // 字符按键2
    for(int i = 0; i < 7; ++i){
        this->symbolKeyMap.insert(QString(':' + i), static_cast<Qt::Key>(Qt::Key_Colon + i));
    }

    // 字符按键3
    for(int i = 0; i < 6; ++i){
        this->symbolKeyMap.insert(QString('[' + i), static_cast<Qt::Key>(Qt::Key_BracketLeft + i));
    }

    // 字符按键4
    for(int i = 0; i < 4; ++i){
        this->symbolKeyMap.insert(QString('{' + i), static_cast<Qt::Key>(Qt::Key_BraceLeft + i));
    }

    // ---------------------控制类按键------------------------
    // 退格
    this->controlKeyMap.insert("Backspace", static_cast<Qt::Key>(Qt::Key_Backspace));


    // 空格
    this->otherKeyMap.insert(QString(' '), static_cast<Qt::Key>(Qt::Key_Space));
}

/**
 * @author  XiaoAn
 * @brief   给按钮绑定按键事件
 * @date    2024-09-07
 */
void KeyBoard::bindKeyEvent()
{
    // 按钮持续按下输入定时器
    this->timerPost = new QTimer(this);
    connect(this->timerPost, &QTimer::timeout, this, [=](){
        QString keyStr = this->timerButton->property("key").toString();
        this->postKeyEvent(keyStr);
        // 重新启动定时器，间隔依次递减
        if(this->timerPost->interval() <= 50){
            this->timerPost->start(50);
        }else{
            this->timerPost->start(this->timerPost->interval() - 50);
        }
    });

    // 为每个按钮按键添加事件
    foreach (auto pair, this->buttonList) {
        QPushButton *button = pair.first;
        // 按钮点击发送按键事件
        connect(button, &QPushButton::clicked, this, [=]() {
            QString keyStr = button->property("key").toString();
            this->postKeyEvent(keyStr);
        });

        // 持续按下按钮，定时触发按键事件
        connect(button, &QPushButton::pressed, this, [=]() {
            this->timerButton = button;
            this->timerPost->start(200);

            // 主动触发窗口按钮事件
            QMouseEvent event(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
            mousePressEvent(&event);
        });

        // 释放按钮，停止定时
        connect(button, &QPushButton::released, this, [=]() {
            this->timerPost->stop();
            // 主动触发事件过滤器
            QMouseEvent event(QEvent::MouseButtonRelease, QPoint(0, 0), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
            mousePressEvent(&event);
        });
    }

    // 切换大小写
    connect(ui->pushButton_switch, &QPushButton::clicked, this, &KeyBoard::switchCaps);

    // 切换符号
    connect(ui->pushButton_symbol, &QPushButton::clicked, this, &KeyBoard::switchSymbol);

    // 关闭键盘
    connect(ui->pushButton_close, &QPushButton::clicked, this, &KeyBoard::close);
}

/**
 * @author  XiaoAn
 * @brief   向输入框发送按键事件
 * @date    2024-09-08
 */
void KeyBoard::postKeyEvent(const QString &keyStr)
{
    // 在四种按键类型中查找
    if(this->numberKeyMap.value(keyStr)){
        // 数字类型
        Qt::Key key = this->numberKeyMap.value(keyStr);
        QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier, keyStr);
        QCoreApplication::postEvent(this->lineEdit, keyEvent);
    }else if(this->letterKeyMap.value(keyStr)){

        // 字符类型
        Qt::Key key = this->letterKeyMap.value(keyStr);
        QChar ch = keyStr.at(0);
        if(ui->pushButton_switch->isChecked()){
            // 小写模式
            ch = ch.toLower();
        }
        QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier, ch);
        QCoreApplication::postEvent(this->lineEdit, keyEvent);
    }else if(this->symbolKeyMap.value(keyStr)){

        // 字符类型
        Qt::Key key = this->symbolKeyMap.value(keyStr);
        QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier, keyStr);
        QCoreApplication::postEvent(this->lineEdit, keyEvent);
    }else if(this->controlKeyMap.value(keyStr)){

        // 控制类型
        Qt::Key key = this->controlKeyMap.value(keyStr);
        QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier);
        QCoreApplication::postEvent(this->lineEdit, keyEvent);
    }else if(this->otherKeyMap.value(keyStr)){

        // 其他
        Qt::Key key = this->otherKeyMap.value(keyStr);
        QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier, keyStr);
        QCoreApplication::postEvent(this->lineEdit, keyEvent);
    }
}


/**
 * @author  XiaoAn
 * @brief   切换大小写
 * @date    2024-09-08
 */
void KeyBoard::switchCaps(bool checked)
{
    if(checked){
        foreach (QPushButton *button , this->switchButton) {
            button->setText(button->text().toLower());
        }
    }else{
        foreach (QPushButton *button , this->switchButton) {
            button->setText(button->text().toUpper());
        }
    }
}

/**
 * @author  XiaoAn
 * @brief   切换到符号按钮
 * @date    2024-09-07
 */
void KeyBoard::switchSymbol(bool checked)
{
    if(checked){

        int symbolIndex = 0;
        QList<QString> symbolList = this->symbolKeyMap.keys();

        // 替换成字符
        for (int i = 0; i < this->switchButton.size(); ++i) {
            QPushButton *button = this->switchButton.at(i);

            // 设置按键属性值
            button->setProperty("key", symbolList.at(symbolIndex));
            // 设置按钮文本
            if(symbolList.at(symbolIndex) == '&'){
                button->setText("&&");
            }else{
                button->setText(symbolList.at(symbolIndex));
            }
            symbolIndex = (symbolIndex + 1) % this->symbolKeyMap.size();
        }
    }else{
        // 恢复成数字字母
        for (int i = 0; i < this->switchButton.size(); ++i) {
            QPushButton *button = this->switchButton.at(i);
            QString str = this->buttonList.at(i).second;

            // 设置按键属性值
            button->setProperty("key", str);

            // 设置按钮文本(大小写转换)
            if(ui->pushButton_switch->isChecked()){
                button->setText(str.toLower());
            }else{
                button->setText(str.toUpper());
            }
        }
    }
}

/**
 * @author  XiaoAn
 * @brief   根据父元素确定合适的摆放位置
 * @date    2024-09-08
 */
QPoint KeyBoard::getPostion()
{
    // 获取输入框基于父控件的坐标
    QPoint lineEditPos = this->lineEdit->mapTo(this->parentWidget(), QPoint(0, 0));

    // 输入框分别在父窗口中、上下左右所间隔的空间
    int bottom = this->parentWidget()->height() - (lineEditPos.y() + this->lineEdit->height());
    int top = lineEditPos.y();
    int right = this->parentWidget()->width() - (lineEditPos.x() + this->lineEdit->width());
    int left = lineEditPos.x();

    if( bottom >= this->height()){
        // 放在输入框下方
        if(right + this->lineEdit->width() >= this->width()){
            // 下右
            return QPoint(lineEditPos.x(), lineEditPos.y() + this->lineEdit->height());
        }else{
            // 下左
            return QPoint(this->parentWidget()->width() - this->width(), lineEditPos.y() + this->lineEdit->height());
        }
    }else if(top >= this->height()){
        // 放在输入框上方
        if(right + this->lineEdit->width() >= this->width()){
            // 上右
            return QPoint(lineEditPos.x(), lineEditPos.y() - this->height());
        }else{
            // 上左
            return QPoint(this->parentWidget()->width() - this->width(), lineEditPos.y() - this->height());
        }
    }else if(right >= this->width()){
        // 放在输入框右侧
        if(bottom + this->lineEdit->height() >= this->height()){
            // 右下
            return QPoint(lineEditPos.x() + this->lineEdit->width(), this->lineEdit->y());
        }else{
            // 右上
            return QPoint(lineEditPos.x() + this->lineEdit->width(), this->parentWidget()->height() - this->height());
        }
    }else if(left >= this->width()){
        // 放在输入框左侧
        if(bottom + this->lineEdit->height() >= this->height()){
            // 左下
            return QPoint(lineEditPos.x() - this->width(), lineEditPos.y());
        }else{
            // 左上
            return QPoint(lineEditPos.x() - this->width(), this->parentWidget()->height() - this->height());
        }
    }else{
        // 居中
        return QPoint(this->parentWidget()->width() / 2 - this->width() / 2,
                      this->parentWidget()->height() / 2 - this->height() / 2);
    }
}


/**
 * @author  XiaoAn
 * @brief   QLineEdit的点击事件过滤器
 * @date    2024-09-06
 */
bool KeyBoard::eventFilter(QObject *obj, QEvent *e)
{
    // 拖拽功能
    if (obj == ui->widget_move) {
        if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            this->isMove = true;
            // 记录鼠标按下的坐标
            this->movePosition = mouseEvent->pos();
        } else if (this->isMove && e->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            // 计算鼠标移动的偏移
            QPoint delta = mouseEvent->pos() - this->movePosition;
            // 移动主窗口
            QPoint newPos = this->pos() + delta;
            // 更新主窗口的位置
            this->move(newPos);
        } else if (e->type() == QEvent::MouseButtonRelease) {
            this->isMove = false;
        }
        return QWidget::eventFilter(obj, e);
    }

    // 鼠标点击事件，键盘窗口的显示与关闭
    if(e->type() == QEvent::MouseButtonPress){

        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(obj);
        // QLineEdit 且 不属于QDateTimeEdit
        if(lineEdit && lineEdit->parent()->metaObject()->className() != QString("QDateTimeEdit")){
            // 获取控件对象
            this->lineEdit = lineEdit;

            // 移动控件到合适位置
            this->move(this->getPostion());

            // 点击显示
            this->show();

            // 设置焦点
            this->lineEdit->setFocus();
        }else{
            this->close();
        }
    }

    // 父窗口改变大小事件
    if(e->type() == QEvent::Resize){
        if(obj == this->parentWidget() && !this->isHidden()){
            this->move(this->getPostion());
        }
    }

    return QWidget::eventFilter(obj, e);
}

/**
 * @author  XiaoAn
 * @brief   鼠标按下事件
 * @date    2024-09-08
 */
void KeyBoard::mousePressEvent(QMouseEvent *e)
{

    this->lastPosition = e->pos();
    this->isMousePressed = true;
}

/**
 * @author  XiaoAn
 * @brief   鼠标松开事件
 * @date    2024-09-08
 */
void KeyBoard::mouseMoveEvent(QMouseEvent *e)
{
    if(!isMousePressed){
        return;
    }
    // 获取滑动的距离
    int dx = e->pos().x() - lastPosition.x();
    int dy = e->pos().y() - lastPosition.y();
    // 判断是否超过 20px
    if (abs(dx) > 20 || abs(dy) > 20) {
        // 检测滑动方向
        if (abs(dx) > abs(dy)) {
            if (dx < 0) {
                // 左
                QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
                QCoreApplication::postEvent(this->lineEdit, keyEvent);
            } else {
                // 右
                QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
                QCoreApplication::postEvent(this->lineEdit, keyEvent);
            }
        } else {
            if (dy < 0) {
                // 上
                QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
                QCoreApplication::postEvent(this->lineEdit, keyEvent);
            } else {
                // 下
                QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
                QCoreApplication::postEvent(this->lineEdit, keyEvent);
            }
        }
        lastPosition = e->pos();  // 更新位置
    }
}

/**
 * @author  XiaoAn
 * @brief   鼠标松开
 * @date    2024-09-08
 */
void KeyBoard::mouseReleaseEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    this->isMousePressed = false;
}


