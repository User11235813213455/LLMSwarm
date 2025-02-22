#ifndef POSITION_HPP_INCLUDED
#define POSITION_HPP_INCLUDED

#include <string>

class Position
{
public:
    /**
     * @brief Constructs a new Position (for an object)
     * 
     * @param pX X component of position of object
     * @param pY Y component of position of object
     * @param pZ Z component of position of object
     * @param pYaw Yaw of object
     */
    Position(double pX, double pY, double pZ, double pYaw);

    /**
    * @brief Constructs a new Position with all coordinates set to 0
    */
    Position();

    /**
     * @brief Converts a position into a string representation "(x,y,z,yaw)"
     * 
     * @return std::string The string representation of the position
     */
    operator std::string() const
    {
        return "(" + std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z) + "," + std::to_string(yaw) + ")";
    }

    /**
     * @brief Checks two positions for equality
     * 
     * @param pOther The other position
     * @return true All components are equal
     * @return false Not all components are equal
     */
    bool operator==(const Position& pOther) const;

    /**
     * @brief Compares two positions by x -> y -> z -> yaw
     * 
     * @param pOther The other position
     * @return true This position is smaller than the other position
     * @return false This position is not smaller than the other position
     */
    bool operator<(const Position& pOther) const;

    /**
     * @brief Adds two positions by adding component wise
     * 
     * @param lhs Left hand side
     * @param rhs Right hand side
     * @return Position Component wise sum
     */
    friend Position operator+(Position lhs, const Position& rhs);


    /**
     * @brief Subtracts two positions by subtracting component wise
     * 
     * @param lhs Left hand side
     * @param rhs Right hand side
     * @return Position Component wise difference
     */
    friend Position operator-(Position lhs, const Position& rhs);

    /**
     * @brief Multiplies all components of a position by a double value
     * 
     * @param lhs Left hand side
     * @param rhs Right hand side
     * @return Position Component wise product
     */
    friend Position operator*(Position lhs, const double& rhs);

    /**
     * @brief Returns the X component
     * 
     * @return double X component
     */
    double getX() const;

    /**
     * @brief Returns the Y component
     * 
     * @return double Y component
     */
    double getY() const;

    /**
     * @brief Returns the Z component
     * 
     * @return double Z component
     */
    double getZ() const;

    /**
     * @brief Returns the yaw component
     * 
     * @return double Yaw component
     */
    double getYaw() const;

    /**
     * @brief Sets the X component
     * 
     * @param pX New X value
     */
    void setX(double pX);

    /**
     * @brief Sets the Y component
     * 
     * @param pY New Y value
     */
    void setY(double pY);

    /**
     * @brief Sets the Z component
     * 
     * @param pZ New Z value
     */
    void setZ(double pZ);

    /**
     * @brief Sets the Yaw component
     * 
     * @param pYaw New yaw value
     */
    void setYaw(double pYaw);

    /**
     * @brief Returns the euclidean distance between two positions
     * 
     * @param pOther The other position
     * @return double Euclidean distance
     */
    double getEuclideanDistance(const Position& pOther) const;

    /**
     * @brief Returns the absolute value of the position vector
     * 
     * @return double Absolute value of the position vector
     */
    double getAbs() const;

protected:
    /**
     * @brief X component of position
     */
    double x;

    /**
     * @brief Y component of position
     */
    double y;

    /**
     * @brief Z component of position
     */
    double z;

    /**
     * @brief Yaw component of position
     */
    double yaw;
};

#endif /*POSITION_HPP_INCLUDED*/