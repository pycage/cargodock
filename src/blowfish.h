#ifndef BLOWFISH_H
#define BLOWFISH_H

#include <QByteArray>
#include <QVector>

class Blowfish
{
public:
    Blowfish(const QByteArray& passphrase);
    QByteArray encrypt(const QByteArray& data) const;
    QByteArray decrypt(const QByteArray& hexData) const;

private:
    void initialize();
    void keySchedule(const QByteArray& passphrase);
    quint32 f(quint32 x) const;
    void encryptLR(quint32& l, quint32& r) const;
    void decryptLR(quint32& l, quint32& r) const;

private:
    QVector<quint32> mySBox0;
    QVector<quint32> mySBox1;
    QVector<quint32> mySBox2;
    QVector<quint32> mySBox3;
    QVector<quint32> myPArray;
};

#endif // BLOWFISH_H
