%% Inverse kinematics
%
% Using the ee position and orientation, determine the joint values.
% 
%%

%% Function: invKin 
% 
% Computes the joint values of the UR robots from their mechanical
% dimensions and end-effector position
% 
% In: d - array with robots dimensions
%     a - array with robots dimensions
%     eePosOri - 4x4 matrix with the position and orientation of the ee
%
% Out: joint - 8x6 matrix with the 8 ik solutions for the 6 joints
%%

function joint=invKin8sol(d, a, eePosOri)
    % 8 inv kin solutions, 6 joints
    ikSol=8;
    numJoints=6;
    joint=zeros(ikSol,numJoints);
    
    %% Computing theta1
    
    % 0P5 position of reference frame {5} in relation to {0}
    P=eePosOri*[0 0 -d(6)-d(7) 1].';
    
    psi=atan2(P(2,1),P(1,1));
    
    % There are two possible solutions for theta1, that depend on whether
    % the shoulder joint (joint 2) is left or right
    phi=acos( (d(2)+d(3)+d(4)+d(5)) / sqrt( P(2,1)^2 + P(1,1)^2 ));

    for i = 1:8
        joint(i,1)=(pi/2+psi-phi)-pi;
    end
    for i = 1:4
        joint(i,1)=(pi/2+psi+phi)-pi;
    end
    
    % From the eePosOri it is possible to know the tipPosOri    
    T_67=MDHMatrix([0 0 d(7) 0]);
    T_06=eePosOri/T_67;

    for j = 1:ikSol
        %% Computing theta5
        
        % Knowing theta1 it is possible to know 0T1
        T_01=MDHMatrix([0 0 d(1) rad2deg(joint(j,1))]);

        % 1T6 = 1T0 * 0T6
        T_16 = T_01\T_06;
        %1P6
        P_16=T_16(:,4);
        
        %Wrist up or down 
        if(ismember(j,[1,2,5,6]))
           joint(j,5)=( (acos( (P_16(2,1)-(d(2)+d(3)+d(4)+d(5) ) ) / d(6)) ) );
        else
            joint(j,5)=(-(acos((P_16(2,1)-(d(2)+d(3)+d(4)+d(5)))/d(6))));
        end

        %% Computing theta 6

         T_61=inv(T_16);
         % y1 seen from frame 6
         Y_16=T_61(:,2);
        % Fix Error using atan2 / Inputs must be real.
            joint=real(double(rad2deg(joint)));
            joint=deg2rad(joint);
         % If theta 5 is equal to zero give arbitrary value to theta 6
        if(int8(rad2deg(real(joint(j,5)))) == 0 || int8(rad2deg(real(joint(j,5)))) == 2*pi)
            joint(j,6) = deg2rad(0);
        else
            joint(j,6) = (pi/2 + atan2( -Y_16(2,1)/sin(joint(j,5))  , Y_16(1,1)/sin(joint(j,5))));
            %joint(j,6) = (atan2( Y_16(2,1)/sin(joint(j,5))  , Y_16(1,1)/sin(joint(j,5))));
        end
        
        %% Computing theta 3, 2 and 4
        
        %Get position of frame 4 from frame 1, P_14
    
        %The value of T_01, T_45, and T_56 are known since joints{1,5,6} have
        %been obtained
        
        %T_45 = T_44'*T_4'5
        T_44_=MDHMatrix([0         a(4)    d(5)   90]);
        T_4_5=MDHMatrix([90        0       0      rad2deg(joint(j,5))]);
        T_45 = T_44_*T_4_5;
        
        %T_56 = T_55'*T_5'6
        T_55_=MDHMatrix([-90        0       0      -90]);
        T_5_6=MDHMatrix([0         a(5)    d(6)   rad2deg(joint(j,6))]);
        T_56 = T_55_*T_5_6;
        
        T_46 = T_45*T_56;
        
        T_64 = inv(T_46);
    
        T_14=T_16*T_64;

        %P_14 is the fourth column of T_14
        P_14=T_14(:,4);

        P_14_xz=sqrt( P_14(1)^2 +  P_14(3)^2);
        
        %Elbow up or down
        if(rem(j,2)==0)
            psi= acos( ( P_14_xz^2-a(3)^2-a(2)^2 )/( -2*a(2)*a(3) ) );
            joint(j,3)=( pi - psi);
            % Masking theta3 for CoppeliaSim (invert value for ang>180)
            if(joint(j,3)>pi)
                joint(j,3)=joint(j,3)-pi*2;
            end
            % theta 2
            % Fix Error using atan2 / Inputs must be real.
            joint=real(double(rad2deg(joint)));
            joint=deg2rad(joint);
            joint(j,2) =pi/2 - (atan2( P_14(3), +P_14(1)) + asin( (a(3)*sin(psi))/P_14_xz));
            % theta 4
            % Fix Error using atan2 / Inputs must be real.
            joint=real(double(rad2deg(joint)));
            joint=deg2rad(joint);
            joint(j,4) = joint4(d, a, joint(j,2), joint(j,3), T_06, T_01, T_64);
        else
            psi= acos( ( P_14_xz^2-a(3)^2-a(2)^2 )/( -2*a(2)*a(3) ) );
            joint(j,3)=( pi + psi);
            % Masking theta3 for CoppeliaSim (invert value for ang>180)
            if(joint(j,3)>pi)
                joint(j,3)=joint(j,3)-pi*2;
            end
            % theta 2
            % Fix Error using atan2 / Inputs must be real.
            joint=real(double(rad2deg(joint)));
            joint=deg2rad(joint);
            joint(j,2) = pi/2 - (atan2( P_14(3), +P_14(1)) + asin( (a(3)*sin(-psi))/P_14_xz));
            % theta 4
            % Fix Error using atan2 / Inputs must be real.
            joint=real(double(rad2deg(joint)));
            joint=deg2rad(joint);
            joint(j,4) = joint4(d, a, joint(j,2), joint(j,3), T_06, T_01, T_64);
        end
    end
    % Fix Error using atan2 / Inputs must be real.
    joint=real(double(rad2deg(joint)));
    joint=deg2rad(joint);
end

%% Computing theta4
function joint4=joint4(d, a, theta2, theta3, T_06, T_01, T_64)

    T_12=MDHMatrix([-90        0       d(2)   rad2deg(theta2)-90]);
    T_23=MDHMatrix([0         a(2)    d(3)   rad2deg(theta3)]);
    T_03=T_01*T_12*T_23;
    
    T_36=T_03\T_06;
    T_34=T_36*T_64;
    
    x_34=T_34(:,1);
    
    joint4 = (atan2(x_34(2),x_34(1)));
end