#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QTimer>

namespace Ui {
class KeyBoard;
}

class KeyBoard : public QWidget
{
    Q_OBJECT

public:
    explicit KeyBoard(QWidget *parent = nullptr);
    ~KeyBoard();

    /// 单例模式初始化
    static KeyBoard* instance(QWidget *parent = nullptr);

private:
    void init();

    /// 初始化按键映射
    void initKeyMap();

    /// 绑定按键事件
    void bindKeyEvent();

    /// 向输入框发送按键事件信息
    void postKeyEvent(const QString &keyStr);

    /// 切换大小写
    void switchCaps(bool checked);

    /// 切换到符号按钮
    void switchSymbol(bool checked);

    /// 获取软键盘合适的位置
    QPoint getPostion();

protected:
    /// 父元素的事件过滤器
    bool eventFilter(QObject *obj, QEvent *e) override;

    /// 鼠标按下事件
    void mousePressEvent(QMouseEvent *e) override;
    /// 鼠标移动事件
    void mouseMoveEvent(QMouseEvent *e) override;
    /// 鼠标松开事件
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    Ui::KeyBoard *ui;

    static KeyBoard *keyBoard;

    /// 聚焦的输入框控件
    QLineEdit *lineEdit = nullptr;

    ///每个按钮默认对应的按键字符串
    QList<QPair<QPushButton*, QString>> buttonList;

    /// 长按定时器
    QTimer *timerPost;
    /// 长按定时器发送的按钮
    QPushButton *timerButton = nullptr;

    /// 用于字符切换的按钮
    QVector<QPushButton*> switchButton;

    /// 1、数字类按键映射
    QMap<QString, Qt::Key> numberKeyMap;
    /// 2、字母类按键映射
    QMap<QString, Qt::Key> letterKeyMap;
    /// 3、字符类按键映射
    QMap<QString, Qt::Key> symbolKeyMap;
    /// 4、控制类按键映射
    QMap<QString, Qt::Key> controlKeyMap;
    /// 5、其他按键映射
    QMap<QString, Qt::Key> otherKeyMap;

    /// 记录鼠标是否按下
    bool isMousePressed = false;
    /// 记录滑动手势的鼠标坐标
    QPoint lastPosition;

    /// 键盘窗口拖拽
    bool isMove = false;
    /// 记录移动时的坐标
    QPoint movePosition;
};

#endif // KEYBOARD_H
