// universalRobotsKinematics.cpp

#include "universalRobotsKinematics.h"
#include <random>


namespace universalRobots
{
	/// <summary>
	/// Constructor. User is only allowed to specify whether there is an end-effector and its translation to the tip.
	/// Example usage:
	///		universalRobots::UR robot_one(); Robot does not have an end-effector.
	///		universalRobots::UR robot_one(true, 0.15f); End-effector translated 0.15 meters from the robot's tip.
	/// </summary>
	/// <param name="endEffector"></param>
	/// <param name="endEffectorDimension"></param>
	UR::UR(const URtype& robotType, const bool& endEffector, const float& endEffectorDimension)
		: m_type(robotType), m_endEffector(endEffector)
	{	
		switch (robotType)
		{
		case universalRobots::URtype::UR3:
			setTransZ(UR3_LINK_DIMENSIONS_d);
			setTransX(UR3_LINK_DIMENSIONS_a);
			break;
		case universalRobots::URtype::UR5:
			setTransZ(UR5_LINK_DIMENSIONS_d);
			setTransX(UR5_LINK_DIMENSIONS_a);
			break;
		case universalRobots::URtype::UR10:
			setTransZ(UR10_LINK_DIMENSIONS_d);
			setTransX(UR10_LINK_DIMENSIONS_a);
			break;
		default:
			setTransZ(UR10_LINK_DIMENSIONS_d);
			setTransX(UR10_LINK_DIMENSIONS_a);
			break;
		}

		m_d[m_numTransZ - 1] = endEffectorDimension;
		m_MDHmatrix <<  0.0f,				0.0f,		m_d[0],			m_jointState[0].m_jointValue,						// 0T1
						mathLib::rad(-90),	0.0f,		m_d[1],			m_jointState[1].m_jointValue + mathLib::rad(-90) ,// 1T2
						0.0f,				m_a[0],		m_d[2],			m_jointState[2].m_jointValue,						// 2T3
						0.0f,				m_a[1],		m_d[3],			m_jointState[3].m_jointValue,						// 3T4
						0.0f,				m_a[2],		m_d[4],			mathLib::rad(90) ,								// 4T4'
						mathLib::rad(90),	0.0f,		0.0f,			m_jointState[4].m_jointValue,						// 4'T5
						mathLib::rad(-90),	0.0f,		0.0f,			mathLib::rad(-90) ,								// 5T5'
						0.0f,				m_a[3],		m_d[5],			m_jointState[5].m_jointValue,						// 5'T6
						0.0f,				0,			m_d[6],			0 ;												// 6T7			
	}
	
	/// <summary>
	/// setRobotType
	/// </summary>
	/// <param name="type"></param>
	void UR::setRobotType(const URtype& type)
	{
		m_type = type;
	}

	/// <summary>
	/// setTransZ
	/// </summary>
	/// <param name="d"></param>
	void UR::setTransZ(const float(&d)[])
	{
		memcpy(m_d, d, sizeof(m_d));
	}
	
	/// <summary>
	/// setTransX
	/// </summary>
	/// <param name="a"></param>
	void UR::setTransX(const float(&a)[])
	{
		memcpy(m_a, a, sizeof(m_a));
	}

	/// <summary>
	/// setMDHmatrix
	///	</summary>
	void UR::setMDHmatrix()
	{
		m_MDHmatrix << 0.0f,					0.0f,		m_d[0],			m_jointState[0].m_jointValue,					// 0T1
						mathLib::rad(-90),		0.0f,		m_d[1],			m_jointState[1].m_jointValue + mathLib::rad(-90) ,// 1T2
						0.0f,					m_a[0],		m_d[2],			m_jointState[2].m_jointValue,					// 2T3
						0.0f,					m_a[1],		m_d[3],			m_jointState[3].m_jointValue,					// 3T4
						0.0f,					m_a[2],		m_d[4],			mathLib::rad(90) ,				// 4T4'
						mathLib::rad(90),		0.0f,		0.0f,			m_jointState[4].m_jointValue,					// 4'T5
						mathLib::rad(-90),		0.0f,		0.0f,			mathLib::rad(-90) ,				// 5T5'
						0.0f,					m_a[3],		m_d[5],			m_jointState[5].m_jointValue,					// 5'T6
						0.0f,					0,			m_d[6],			0 ;								// 6T7
	}
	
	/// <summary>
	/// setTheta
	/// </summary>
	/// <param name="jointVal"></param>
	void UR::setTheta(const float(&jointVal)[])
	{
		for (unsigned int i = 0; i < m_numDoF; i++)
			m_jointState[i].m_jointValue = jointVal[i];
	} 
	
	/// <summary>
	/// Returns the enum URtype. Used for printing purposes.
	/// </summary>
	/// <returns>m_type</returns>
	const URtype UR::getRobotType() const
	{
		return m_type;
	}
	
	/// <summary>
	/// Returns the values of the z-axis translations (d array). Used for printing purposes.
	/// </summary>
	/// <returns>m_d</returns>
	const float* UR::getTransZ() const
	{
		return m_d;
	}
	
	/// <summary>
	/// Returns the values of the x-axis translations (a array). Used for printing purposes.
	/// </summary>
	/// <returns>m_a</returns>
	const float* UR::getTransX() const
	{
		return m_a;
	}
	
	/// <summary>
	/// Returns the robot's current joint values. Used for printing purposes.
	/// </summary>
	/// <returns>m_theta</returns>
	const float UR::getTheta(const int &ix) const
	{
		return m_jointState[ix].m_jointValue;
	}
	
	/// <summary>
	/// Returns the robot's current tip pose. Used for printing purposes.
	/// </summary>
	/// <returns>m_tipPose</returns>
	const pose UR::getTipPose() const
	{
		return m_jointState[m_numDoF-1].m_jointPose;
	}
	
	/// <summary>
	/// Receives an array of target joint values and computes the pose of the robot's tip.
	/// </summary>
	/// <param name="targetJointVal"></param>
	/// <returns>tipPose</returns>
	pose UR::forwardKinematics(const float(&targetJointVal)[])
	{
		// Assign joint values to compute MDH matrix.
		setTheta(targetJointVal);
		setMDHmatrix();

		// Determine the indiviual transformation matrices.
		for (unsigned int i = 0; i < m_numReferenceFrames; i++)
			m_individualTransformationMatrices[i] = mathLib::calcTransformationMatrix(m_MDHmatrix.row(i));

		// Determine the general transformation matrices.		
		Eigen::Matrix3f rotationMatrix;
		unsigned int currentJoint = 0;
		pose tipPose = {};
		for (unsigned int i = 0; i < m_numReferenceFrames; i++)
		{
			if(!i)
				m_generalTransformationMatrices[0] = m_individualTransformationMatrices[0];
			else
				m_generalTransformationMatrices[i] = m_generalTransformationMatrices[i - 1] * m_individualTransformationMatrices[i];


			// Obtaining the joint pose
			// Since we have more reference frames than joints only some represent a joint pose
			// 0T1 J1 // 1T2 J2// 2T3 J3// 3T4 J4// 4T4' XX// 4'T5 J5// 5T5' XX// 5'T6 J6// 6T7 tipPose
			if (i == 0 || i == 1 || i == 2 || i == 3 || i == 5 || i == 7 || i == 8)
			{
				// 0T1 1 // 1T2 2// 2T3 3// 3T4 4// 4T4' // 4'T5 5// 5T5' // 5'T6 6// 6T7
				rotationMatrix << m_generalTransformationMatrices[i](0, 0), m_generalTransformationMatrices[i](0, 1), m_generalTransformationMatrices[i](0, 2),
					m_generalTransformationMatrices[i](1, 0), m_generalTransformationMatrices[i](1, 1), m_generalTransformationMatrices[i](1, 2),
					m_generalTransformationMatrices[i](2, 0), m_generalTransformationMatrices[i](2, 1), m_generalTransformationMatrices[i](2, 2);

				float position[3] = { m_generalTransformationMatrices[i](0, 3), m_generalTransformationMatrices[i](1, 3), m_generalTransformationMatrices[i](2, 3) };

				// Tip pose
				if (i == 8)
					tipPose = pose(position, rotationMatrix);
				else
				{
					m_jointState[currentJoint].m_jointPose = pose(position, rotationMatrix);
					currentJoint++;
				}
			}
		}

		//return m_jointState[m_numDoF-1].m_jointPose;
		return tipPose;
	}
	
	/// <summary>
	/// Computes the eight inverse kinematics solutions for a given tip pose.
	/// </summary>
	/// <param name="targetTipPose"></param>
	/// <param name="outIkSols"></param>
	void UR::inverseKinematics(const pose &targetTipPose, float (*outIkSols)[m_numIkSol][m_numDoF])
	{
		Eigen::Matrix4f T_07 = Eigen::Matrix4f::Identity(); // 0T7

		// Get translation matrix.
		const Eigen::Vector3f translation = { targetTipPose.m_pos[0], targetTipPose.m_pos[1], targetTipPose.m_pos[2] };

		// Calculate rotation matrix.
		Eigen::Matrix3f rotation;
		rotation = Eigen::AngleAxisf(targetTipPose.m_eulerAngles[0], Eigen::Vector3f::UnitX())
				*Eigen::AngleAxisf(targetTipPose.m_eulerAngles[1], Eigen::Vector3f::UnitY())
				*Eigen::AngleAxisf(targetTipPose.m_eulerAngles[2], Eigen::Vector3f::UnitZ());

		T_07.block<3, 3>(0, 0) = rotation;
		T_07.block<3, 1>(0, 3) = translation;

		// Computing theta1.
		const Eigen::Matrix<float, 1, 4> P_05 = T_07 * Eigen::Matrix<float, 1, 4>(0.0f, 0.0f, -m_d[5] - m_d[6], 1.0f).transpose(); // 0P5 position of reference frame {5} in relation to {0}
		const float theta1_psi = atan2(P_05(0, 1), P_05(0, 0));

		// There are two possible solutions for theta1, that depend on whether the shoulder joint (joint 2) is left or right.
		const float theta1_phi = acos( (m_d[1] + m_d[2] + m_d[3] + m_d[4]) / (sqrt( pow(P_05(0, 1), 2) + pow(P_05(0, 0), 2) ) ) );

		for (int i = -4; i < int (m_numIkSol) - 4 ; i++)
		{
			(*outIkSols)[i + 4][0] = (std::numbers::pi_v<float> / 2 + theta1_psi + (i < 0 ? 1 : -1)* theta1_phi) - std::numbers::pi_v<float>; // (i < 0 ? 1 : -1) first 4 theta1 values have positive phi
		}
		
		Eigen::Matrix4f T_06 = T_07;
		T_06(2, 3) = T_07(2, 3) - m_d[m_numTransZ - 1]; // 0T6

		for (unsigned int i = 0; i < m_numIkSol; i++)
		{
			// Computing theta5.
			 Eigen::Matrix4f T_01 = mathLib::calcTransformationMatrix(Eigen::RowVector4f{ 0.0f, 0.0f, m_d[0], (*outIkSols)[i][0] }); // Knowing theta1 it is possible to know 0T1
			 Eigen::Matrix4f T_16 = T_01.inverse() * T_06; // 1T6 = 1T0 * 0T6
			// There are two possible solutions for theta5, that depend on whether the wrist joint is up or down.
			if (i == 0 || i == 1 || i == 4 || i == 5) //(0, 1, 4, 5)
				(*outIkSols)[i][4] =  acos( (T_16(1, 3) - (m_d[1] + m_d[2] + m_d[3] + m_d[4])) / m_d[5] );
			else
				(*outIkSols)[i][4] = - acos( (T_16(1, 3) - (m_d[1] + m_d[2] + m_d[3] + m_d[4])) / m_d[5] );

			// Computing theta6.
			if ((*outIkSols)[i][4] == 0 || (*outIkSols)[i][4] == 2 * std::numbers::pi_v<float>) // If theta5 is equal to zero.
				(*outIkSols)[i][5] = 0.0f; // Give arbitrary value to theta6
			else
			{
				const float sinTheta5 = sin((*outIkSols)[i][4]);
				(*outIkSols)[i][5] = std::numbers::pi_v<float> / 2 + atan2(-T_16.inverse()(1, 1) / sinTheta5, T_16.inverse()(0, 1) / sinTheta5);

			}
				

			// Computing theta3, theta2, and theta4.

			// T_45 = T_44'*T_4'5
			 Eigen::Matrix4f T_44_ = mathLib::calcTransformationMatrix(Eigen::RowVector4f (0.0f, m_a[2], m_d[4], std::numbers::pi_v<float> / 2));
			 Eigen::Matrix4f T_4_5 = mathLib::calcTransformationMatrix(Eigen::RowVector4f(std::numbers::pi_v<float> / 2, 0.0f, 0.0f, (*outIkSols)[i][4]));
			 Eigen::Matrix4f T_45 = T_44_ * T_4_5;

			// T_56 = T_55'*T_5'6
			 Eigen::Matrix4f T_55_ = mathLib::calcTransformationMatrix(Eigen::RowVector4f(-std::numbers::pi_v<float> / 2, 0.0f, 0.0f, -std::numbers::pi_v<float> / 2));
			 Eigen::Matrix4f T_5_6 = mathLib::calcTransformationMatrix(Eigen::RowVector4f(0.0f, m_a[3], m_d[5], (*outIkSols)[i][5]));
			 Eigen::Matrix4f T_56 = T_55_ * T_5_6;

			 Eigen::Matrix4f T_46 = T_45 * T_56;
			 Eigen::Matrix4f T_14 = T_16 * T_46.inverse();

			 float P_14_xz = sqrtf( pow(T_14(0, 3), 2) + pow(T_14(2, 3), 2));
			 float theta3_psi = acos((pow(P_14_xz, 2) - pow(m_a[1], 2) - pow(m_a[0], 2)) / (-2 * m_a[0] * m_a[1]));

			// Elbow up or down
			if ((i + 1) % 2 == 0)
			{
				// Computing theta3.
				(*outIkSols)[i][2] = std::numbers::pi_v<float> - theta3_psi;
				// Masking theta3 for CoppeliaSim(invert value for ang > 180).
				if ((*outIkSols)[i][2] > std::numbers::pi_v<float>)
					(*outIkSols)[i][2] = (*outIkSols)[i][2] - std::numbers::pi_v<float> * 2;
				// Computing theta2.
				(*outIkSols)[i][1] = std::numbers::pi_v<float> / 2 - atan2(T_14(2, 3), T_14(0, 3)) + asin((m_a[1] * sin(-theta3_psi)) / P_14_xz);
				// Computing theta4.
				 Eigen::Matrix4f T_12 = mathLib::calcTransformationMatrix(Eigen::RowVector4f(-std::numbers::pi_v<float> / 2, 0.0f, m_d[1], (*outIkSols)[i][1] - std::numbers::pi_v<float> / 2));
				 Eigen::Matrix4f T_23 = mathLib::calcTransformationMatrix(Eigen::RowVector4f(0.0f, m_a[0], m_d[2], (*outIkSols)[i][2]));
				 Eigen::Matrix4f T_03 = T_01 * T_12 * T_23;

				 Eigen::Matrix4f T_36 = T_03.inverse() * T_06;
				 Eigen::Matrix4f T_34 = T_36 * T_46.inverse();

				 (*outIkSols)[i][3] = atan2(T_34(1, 0), T_34(0, 0));
			}
			else
			{
				// Computing theta3.
				(*outIkSols)[i][2] = std::numbers::pi_v<float> + theta3_psi;
				// Masking theta3 for CoppeliaSim(invert value for ang > 180).
				if ((*outIkSols)[i][2] > std::numbers::pi_v<float>)
					(*outIkSols)[i][2] = (*outIkSols)[i][2] - std::numbers::pi_v<float> *2;
				// Computing theta2.
				(*outIkSols)[i][1] = std::numbers::pi_v<float> / 2 - atan2(T_14(2, 3), T_14(0, 3)) + asin(m_a[1] * sin(theta3_psi) / P_14_xz);
				// Computing theta4.
				 Eigen::Matrix4f T_12 = mathLib::calcTransformationMatrix(Eigen::RowVector4f(-std::numbers::pi_v<float> / 2, 0.0f, m_d[1], (*outIkSols)[i][1] - std::numbers::pi_v<float> / 2));
				 Eigen::Matrix4f T_23 = mathLib::calcTransformationMatrix(Eigen::RowVector4f(0.0f, m_a[0], m_d[2], (*outIkSols)[i][2]));
				 Eigen::Matrix4f T_03 = T_01 * T_12 * T_23;

				 Eigen::Matrix4f T_36 = T_03.inverse() * T_06;
				 Eigen::Matrix4f T_34 = T_36 * T_46.inverse();

				 (*outIkSols)[i][3] = atan2(T_34(1, 0), T_34(0, 0));
			}
		}
	}
	
	/// <summary>
	/// Generates a valid tip pose by running forward kinematics with random target joint values.
	/// </summary>
	/// <returns>randomValidTargetPose</returns>
	pose UR::generateRandomReachablePose() 
	{
		//// https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(-360, 360); // URs joints limits [-360; 360]

		float randomTargetJointValue[m_numDoF] = {};
		
		for (unsigned int i = 0; i < m_numDoF; i++)
		{
			randomTargetJointValue[i] = mathLib::rad(distrib(gen));
		}

		return forwardKinematics(randomTargetJointValue);

		 
	}


	/// <summary>
	/// Check the reachability of a target tip pose.
	/// </summary>
	/// <param name="targetTipPose"></param>
	/// <returns>bool</returns>
	bool UR::checkPoseReachability(const float(&ikSol)[]) const
	{
		for (unsigned int i = 0; i < 6; i++)
		{
			if(isnan(ikSol[i]))
				return false;
		}

		return true;
	}

	/// <summary>
	/// Operator overloading to be able to print a URtype enum.
	/// </summary>
	/// <param name="stream"></param>
	/// <param name="type"></param>
	/// <returns>stream</returns>
	std::ostream& operator <<(std::ostream& stream, const universalRobots::URtype& type)
	{
		switch (type)
		{
		case universalRobots::URtype::UR3:
			stream << "UR3";
			break;
		case universalRobots::URtype::UR5:
			stream << "UR5";
			break;
		case universalRobots::URtype::UR10:
			stream << "UR10";
			break;
		default:
			stream.setstate(std::ios_base::failbit);
		}
		return stream;
	}

	/// <summary>
	/// Operator overloading to be able to print a UR object.
	/// </summary>
	/// <param name="stream"></param>
	/// <param name="robot"></param>
	/// <returns>stream</returns>
	std::ostream& operator <<(std::ostream& stream, const universalRobots::UR& robot)
	{
		stream << "Robot type: " << robot.m_type << std::endl
			<< "Number of DoFs: " << robot.m_numDoF << std::endl
			<< "Link dimensions\n" << "Translations in the z-axis (meters):\n";
		for (unsigned int i = 0; i < robot.m_numTransZ; i++)
			stream << "d" << i + 1 << ": " << robot.m_d[i] << std::endl;
		stream << "Translations in the x-axis (meters):\n";
		for (unsigned int i = 0; i < robot.m_numTransX; i++)
			stream << "a" << i + 2 << ": " << robot.m_a[i] << std::endl;
		//stream << "Modified Denavit-Hartengerg Matrix\n";
		//for (int x = 0; x < robot.m_numReferenceFrames; x++)  // loop lines
		//{
		//	for (int y = 0; y < 4; y++)  // loop columns
		//	{
		//		//stream << robot.getMDHmatrix()[x][y];
		//		stream << (robot.m_MDHmatrix[x][y]) << "				";
		//	}
		//	stream << endl;  
		//}
		stream << "Joint values (degrees):\n";
		for (unsigned int i = 0; i < robot.m_numDoF; i++)
			stream << "Theta" << i + 1 << ": " << mathLib::deg(robot.getTheta(i)) << std::endl;
		stream << "Tip pose:\n";
		const pose tipPose = robot.getTipPose();
		stream << "x " << tipPose.m_pos[0] << " y " << tipPose.m_pos[1] << " z " << tipPose.m_pos[2] << " (meters)\nalpha "
				<< mathLib::deg(tipPose.m_eulerAngles[0]) << " beta " << mathLib::deg(tipPose.m_eulerAngles[1]) << " gamma " << mathLib::deg(tipPose.m_eulerAngles[2]) << " (degrees)" << std::endl;
		stream << "Individual Transformation Matrices:\n";
		unsigned int counter = 0;
		for (unsigned int i = 0; i < robot.m_numReferenceFrames; i++)
		{
			if (i == 0 || i == 1 || i == 2 || i == 3 || i == 8)
			{
				stream << counter << "T" << counter + 1 << std::endl << robot.m_individualTransformationMatrices[i] << std::endl;
				counter++;
			}	
			else
			{
				if(i == 4)
					stream << counter << "T" << counter << "'" << std::endl << robot.m_individualTransformationMatrices[i] << std::endl;
				if (i == 5)
				{					
					stream << counter << "'T" << counter + 1 << std::endl << robot.m_individualTransformationMatrices[i] << std::endl;
					counter++;
				}
				if (i == 6)
					stream << counter << "T" <<  counter << "'" <<std::endl << robot.m_individualTransformationMatrices[i] << std::endl;
				if (i == 7)
				{
					stream << counter << "'T" << counter + 1 << std::endl << robot.m_individualTransformationMatrices[i] << std::endl;
					counter++;
				}
			}
		}
		stream << "General Transformation Matrices:\n";
		counter = 1;
		for (unsigned int i = 0; i < robot.m_numReferenceFrames; i++)
		{
			if (i == 0 || i == 1 || i == 2 || i == 3 || i == 5 || i == 7 || i == 8)
			{
				stream << "0T" << counter << std::endl << robot.m_generalTransformationMatrices[i] << std::endl;
				counter++;
			}
				
			else
			{
				if (i == 4)
					stream << "0T" << counter - 1 << "'" << std::endl << robot.m_generalTransformationMatrices[i] << std::endl;
				if (i == 6)
					stream << "0T" << counter - 1 << "'" << std::endl << robot.m_generalTransformationMatrices[i] << std::endl;
			}
		}
		stream << "Joint poses: {x, y, z} metres {alpha, beta, gamma} degrees\n";
		for (unsigned int i = 0; i < robot.m_numDoF; i++)
			stream << "J" << i + 1	<< ": {" << robot.m_jointState[i].m_jointPose.m_pos[0]  << ", " << robot.m_jointState[i].m_jointPose.m_pos[1] << ", " << robot.m_jointState[i].m_jointPose.m_pos[2] << "}"
									<< " {" << mathLib::deg(robot.m_jointState[i].m_jointPose.m_eulerAngles[0]) << ", " << mathLib::deg(robot.m_jointState[i].m_jointPose.m_eulerAngles[1]) << ", " << mathLib::deg(robot.m_jointState[i].m_jointPose.m_eulerAngles[2]) << "}" << std::endl;

		return stream;
	}

} // namespace universalRobots 