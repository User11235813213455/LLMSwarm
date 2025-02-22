#include "position.hpp"
#include <cmath>

Position::Position(double pX, double pY, double pZ, double pYaw)
: x(pX), y(pY), z(pZ), yaw(pYaw)
{

}
Position::Position()
: x(0), y(0), z(0), yaw(0)
{

}

double Position::getX() const
{
    return this->x;
}
double Position::getY() const
{
    return this->y;
}
double Position::getZ() const
{
    return this->z;
}
double Position::getYaw() const
{
    return this->yaw;
}

void Position::setX(double pX)
{
    this->x = pX;
}
void Position::setY(double pY)
{
    this->y = pY;
}
void Position::setZ(double pZ)
{
    this->z = pZ;
}
void Position::setYaw(double pYaw)
{
    this->yaw = pYaw;
}
bool Position::operator==(const Position& pOther) const
{
    return this->x == pOther.x && this->y == pOther.y && this->z == pOther.z && this->yaw == pOther.yaw;
}
bool Position::operator<(const Position& pOther) const
{
    if(this->x < pOther.x)
    {
        return true;
    }
    else if(this->x > pOther.x)
    {
        return false;
    }
    else
    {
        if(this->y < pOther.y)
        {
            return true;
        }
        else if(this->y > pOther.y)
        {
            return false;
        }
        else
        {
            if(this->z < pOther.z)
            {
                return true;
            }
            else if(this->z > pOther.z)
            {
                return false;
            }
            else
            {
                if(this->yaw < pOther.yaw)
                {
                    return true;
                }
                else if(this->yaw > pOther.yaw)
                {
                    return false;
                }
                else
                {
                    /*Equal*/
                    return false;
                }
            }   
        }
    }
}
double Position::getEuclideanDistance(const Position& pOther) const
{
    return (pOther - *this).getAbs();
}
Position operator+(Position lhs, const Position& rhs)
{
    lhs.x = lhs.x + rhs.x;
    lhs.y = lhs.y + rhs.y;
    lhs.z = lhs.z + rhs.z;
    lhs.yaw = lhs.yaw + rhs.yaw;
    return lhs;
}
Position operator-(Position lhs, const Position& rhs)
{
    lhs.x = lhs.x - rhs.x;
    lhs.y = lhs.y - rhs.y;
    lhs.z = lhs.z - rhs.z;
    lhs.yaw = lhs.yaw - rhs.yaw;
    return lhs;
}
double Position::getAbs() const
{
    return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
}
Position operator*(Position lhs, const double& rhs)
{
    lhs.x = lhs.x * rhs;
    lhs.y = lhs.y * rhs;
    lhs.z = lhs.z * rhs;
    lhs.yaw = lhs.yaw * rhs;
    return lhs;
}