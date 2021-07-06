#include "tocabi_controller/tocabi_controller.h"

using namespace std;

TocabiController::TocabiController(StateManager &stm_global) : dc_(stm_global.dc_)
,stm_(stm_global)
,rd_(stm_global.dc_.rd_)
#ifdef COMPILE_TOCABI_CC
,my_cc(*(new CustomController(rd_)))
#endif
#ifdef COMPILE_TOCABI_AVATAR
,ac_(*(new AvatarController(rd_)))
#endif
{
    //Tocabi Controller Initialize Component

    nh_controller_.setCallbackQueue(&queue_controller_);
    //sub_1 = nh_controller_.subscribe("/tocabi/avatar_test", 1, &AvatarController::avatar_callback, this);

    task_command_sub_ = nh_controller_.subscribe("/tocabi/taskcommand", 100, &TocabiController::TaskCommandCallback, this);
    task_command_que_sub_ = nh_controller_.subscribe("/tocabi/taskquecommand", 100, &TocabiController::TaskQueCommandCallback, this);
    position_command_sub_ = nh_controller_.subscribe("/tocabi/positioncommand", 100, &TocabiController::PositionCommandCallback, this);
}

TocabiController::~TocabiController()
{
    cout << "TocabiController Terminated" << endl;
}

// Thread1 : running
void *TocabiController::Thread1() //Thread1, running with 2Khz.
{
    std::cout << "thread1_entered" << std::endl;

    volatile int rcv_time_ = 0;
    //cout << "shm_msgs:" << dc_.tc_shm_->t_cnt << endl;
    //cout << "entered" << endl;

    if (dc_.tc_shm_->shutdown)
    {
        cout << "what?" << endl;
    }

    //cout << "waiting first calc.." << endl;
    while (!rd_.firstCalc)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        if (dc_.tc_shm_->shutdown)
        {
            break;
        }
    }

    std::cout << "thread1 Proceeding ... " << endl;

    WBC::SetContactInit(rd_);

    EnableThread2(true);  //Set true for Thread2
    EnableThread3(false); //True for thread3 ...

    //std::cout<<"21"<<std::endl;

    std::cout << "entering thread1 loop" << endl;

    signalThread1 = true;
    int thread1_count = 0;
    while (!dc_.tc_shm_->shutdown)
    {
        if (dc_.triggerThread1)
        {
            dc_.triggerThread1 = false;
            thread1_count++;
            if (dc_.tc_shm_->shutdown)
                break;
            rcv_time_ = rd_.us_from_start_;

            auto t1 = std::chrono::steady_clock::now();

            //////////////////////////////////////////////////////////
            ////////////////////Start Tocabi Controll/////////////////
            //////////////////////////////////////////////////////////

            queue_controller_.callAvailable(ros::WallDuration());

            if (rd_.task_signal_ || rd_.task_que_signal_)
            {
                std::cout << "task signal received mode :" << rd_.tc_.mode << std::endl;
                rd_.tc_time_ = rd_.control_time_;
                rd_.tc_run = true;
                rd_.tc_init = true;
                rd_.link_[Right_Foot].SetInitialWithPosition();
                rd_.link_[Left_Foot].SetInitialWithPosition();
                rd_.link_[Right_Hand].SetInitialWithPosition();
                rd_.link_[Left_Hand].SetInitialWithPosition();
                rd_.link_[Pelvis].SetInitialWithPosition();
                rd_.link_[Upper_Body].SetInitialWithPosition();
                rd_.link_[COM_id].SetInitialWithPosition();

                double pos_p = 400.0;
                double pos_d = 40.0;
                double pos_a = 1;
                double rot_p = 400.0;
                double rot_d = 40.0;
                double rot_a = 1.0;

                rd_.link_[Right_Foot].SetGain(pos_p, pos_d, pos_a, rot_p, rot_d, rot_a);
                rd_.link_[Left_Foot].SetGain(pos_p, pos_d, pos_a, rot_p, rot_d, rot_a);
                rd_.link_[Right_Hand].SetGain(pos_p, pos_d, pos_a, rot_p, rot_d, rot_a);
                rd_.link_[Left_Hand].SetGain(pos_p, pos_d, pos_a, rot_p, rot_d, rot_a);
                rd_.link_[Pelvis].SetGain(pos_p, pos_d, pos_a, rot_p, rot_d, rot_a);
                rd_.link_[Upper_Body].SetGain(pos_p, pos_d, pos_a, rot_p, rot_d, rot_a);
                rd_.link_[COM_id].SetGain(pos_p, pos_d, pos_a, rot_p, rot_d, rot_a);

                std::cout << " pelv yaw init : " << rd_.link_[Pelvis].yaw_init << std::endl;

                std::cout << "upperbody rotation init : " << DyrosMath::rot2Euler_tf(rd_.link_[Upper_Body].rot_init).transpose() << std::endl;

                if (rd_.task_signal_)
                {
                    rd_.task_signal_ = false;
                }

                if (rd_.task_que_signal_)
                {
                    std::cout << "task que received ... but doing nothing .." << std::endl;
                    rd_.task_que_signal_ = false;
                }

                if (!rd_.semode)
                {
                    std::cout << "State Estimate is not running. disable task command" << std::endl;
                    rd_.tc_run = false;
                }
            }

            VectorQd torque_task_, torque_grav_, torque_contact_;
            torque_task_.setZero();
            torque_grav_.setZero();
            torque_contact_.setZero();

            if (rd_.tc_run)
            {
                if (rd_.tc_.mode == 0)
                {
                    if (rd_.tc_init)
                    {
                        std::cout << "mode 0 init" << std::endl;
                        rd_.tc_init = false;

                        rd_.link_[COM_id].x_desired = rd_.link_[COM_id].x_init;
                    }

                    WBC::SetContact(rd_, 1, 1);

                    rd_.J_task.setZero(9, MODEL_DOF_VIRTUAL);
                    rd_.J_task.block(0, 0, 6, MODEL_DOF_VIRTUAL) = rd_.link_[COM_id].Jac();
                    rd_.J_task.block(6, 0, 3, MODEL_DOF_VIRTUAL) = rd_.link_[Upper_Body].Jac().block(3, 0, 3, MODEL_DOF_VIRTUAL);

                    rd_.link_[COM_id].x_desired = rd_.tc_.ratio * rd_.link_[Left_Foot].x_init + (1 - rd_.tc_.ratio) * rd_.link_[Right_Foot].x_init;
                    rd_.link_[COM_id].x_desired(2) = rd_.tc_.height;

                    rd_.link_[Upper_Body].rot_desired = DyrosMath::Euler2rot(rd_.tc_.roll, rd_.tc_.pitch, rd_.tc_.yaw + rd_.link_[Pelvis].yaw_init);

                    Eigen::VectorXd fstar;
                    rd_.link_[COM_id].SetTrajectoryQuintic(rd_.control_time_, rd_.tc_time_, rd_.tc_time_ + rd_.tc_.time);
                    rd_.link_[Upper_Body].SetTrajectoryRotation(rd_.control_time_, rd_.tc_time_, rd_.tc_time_ + rd_.tc_.time);

                    fstar.setZero(9);
                    fstar.segment(0, 6) = WBC::GetFstar6d(rd_.link_[COM_id]);
                    fstar.segment(6, 3) = WBC::GetFstarRot(rd_.link_[Upper_Body]);

                    rd_.torque_desired = WBC::ContactForceRedistributionTorque(rd_, WBC::GravityCompensationTorque(rd_) + WBC::TaskControlTorque(rd_, fstar));

                    /*
                    auto ts = std::chrono::steady_clock::now();
                    WBC::GetJKT1(rd_, rd_.J_task);
                    auto ds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - ts).count();

                    auto ts2 = std::chrono::steady_clock::now();
                    WBC::GetJKT2(rd_, rd_.J_task);
                    auto ds2 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - ts2).count();

                    rd_.time_for_inverse += ds;
                    rd_.time_for_inverse_total += ds2;

                    rd_.count_for_inverse++;
                    rd_.count_for_inverse_total++;

                    if (rd_.count_for_inverse == 2000)
                    {
                        std::cout << "avg 1 : " << rd_.time_for_inverse / rd_.count_for_inverse << " 2 : " << rd_.time_for_inverse_total / rd_.count_for_inverse_total << std::endl;

                        rd_.time_for_inverse = 0;
                        rd_.time_for_inverse_total = 0;
                        rd_.count_for_inverse = 0;
                        rd_.count_for_inverse_total = 0;
                    }*/
                }

#ifdef COMPILE_TOCABI_AVATAR
                if ((rd_.tc_.mode > 9) && (rd_.tc_.mode < 15))
                {
                    RequestThread2();
                    ac_.computeSlow();

                    //If necessary, use
                    //To Enable Thread2, you need to fix the 50th line. Change EnableThread2(false) to EnableThread2(true).
                    //If not, thread2 is disabled, so that you cannot use thread2
                    //RequestThread2() : call this function to trigger Thread2 at each tick.
                }
#endif
#ifdef COMPILE_TOCABI_CC
                if (rd_.tc_.mode == 15)
                {
                    RequestThread2();
                    my_cc.computeSlow();
                }
#endif
            }
            else
            {
                WBC::SetContact(rd_, 1, 1);
                WBC::GravityCompensationTorque(rd_);
                rd_.torque_desired = WBC::ContactForceRedistributionTorque(rd_, WBC::GravityCompensationTorque(rd_));
            }

            //Send Data To thread2

            //Data2Thread2

            //std::cout << torque_task_.norm() << "\t" << torque_grav_.norm() << "\t" << torque_contact_.norm() << std::endl;

            static std::chrono::steady_clock::time_point t_c_ = std::chrono::steady_clock::now();

            SendCommand(rd_.torque_desired);

            auto t_end = std::chrono::steady_clock::now();

            auto d1 = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t1).count();            //150us without march=native
            auto d2 = std::chrono::duration_cast<std::chrono::microseconds>(t_end - rd_.tp_state_).count(); //150us without march=native

            static int d1_over_cnt = 0;

            if (d1 > 500)
            {
                d1_over_cnt++;
            }

            static int d2_total = 0;
            static double d1_total = 0;

            d2_total += d2;
            d1_total += d1;

            // if (d2 > 350)
            // {
            //     std::cout << rd_.control_time_ << "command duration over 350us , " << d2 << std::endl;
            // }

            if (thread1_count % 2000 == 0)
            {
                /*
                WBC::SetContact(rd_, 1, 1);

                WBC::SetContact(rd_, 1, 0);

                WBC::SetContact(rd_, 0, 1);*/

                // std::cout << rd_.control_time_ << "s : avg rcv2send : " << d2_total / thread1_count << " us, state : " << rd_.state_ctime_total_ / thread1_count << " controller : " << d1_total / thread1_count << " diff : " << (d2_total - rd_.state_ctime_total_ - d1_total) / thread1_count << std::endl;

                if (d1_over_cnt > 0)
                {
                    std::cout << cred << "Controller Thread1 calculation time over 500us.. : " << d1_over_cnt << "times" << creset << std::endl;
                    d1_over_cnt = 0;
                }

                d1_total = 0;
                d2_total = 0;
                rd_.state_ctime_total_ = 0;
                thread1_count = 0;
            }
            t_c_ = std::chrono::steady_clock::now();

            //std::cout<<"21"<<std::endl;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }

    cout << "thread1 terminate" << endl;
}

//Thread2 : running with request
void *TocabiController::Thread2()
{
    while (true)
    {
        if (signalThread1 || dc_.tc_shm_->shutdown)
            break;
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    if (enableThread2)
    {
        std::cout << "thread2_entered" << std::endl;
        while (!dc_.tc_shm_->shutdown)
        {
            if (triggerThread2)
            {
                triggerThread2 = false;
                /////////////////////////////////////////////
                /////////////Do something in Thread2 !!!!!!!

                if (rd_.tc_run)
                {
#ifdef COMPILE_TOCABI_AVATAR
                    if ((rd_.tc_.mode > 9) && (rd_.tc_.mode < 15))
                    {
                        ac_.computeFast();
                    }
#endif
#ifdef COMPILE_TOCABI_CC
                    if (rd_.tc_.mode == 15)
                    {
                        my_cc.computeFast();
                    }
#endif
                }
                /////////////////////////////////////////////
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        }
    }
    else
    {
        std::cout << "thread2 disabled" << std::endl;
    }

    std::cout << "thread2 terminate" << std::endl;
}

//Thread3 : running with request
void *TocabiController::Thread3()
{
    while (true)
    {
        if (signalThread1 || dc_.tc_shm_->shutdown)
            break;

        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    if (enableThread3)
    {
        std::cout << "thread3_entered" << std::endl;

        while (!dc_.tc_shm_->shutdown)
        {
            if (triggerThread3)
            {
                triggerThread3 = false;
                /////////////////////////////////////////////
                /////////////Do something in Thread3 !!!!!!!

                /////////////////////////////////////////////
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        }
    }
    else
    {
        std::cout << "thread3 disabled" << std::endl;
    }

    std::cout << "thread3 terminate" << std::endl;
}

void TocabiController::MeasureTime(int currentCount, int nanoseconds1, int nanoseconds2)
{
    dc_.tc_shm_->t_cnt2 = currentCount;

    lat = nanoseconds1;
    total1 += lat;
    lavg = total1 / currentCount;
    if (lmax < lat)
    {
        lmax = lat;
    }
    if (lmin > lat)
    {
        lmin = lat;
    }
    // int sdev = (sat - savg)
    total_dev1 += sqrt(((lat - lavg) * (lat - lavg)));
    ldev = total_dev1 / currentCount;

    dc_.tc_shm_->lat_avg2 = lavg;
    dc_.tc_shm_->lat_max2 = lmax;
    dc_.tc_shm_->lat_min2 = lmin;
    dc_.tc_shm_->lat_dev2 = ldev;

    sat = nanoseconds2;
    total2 += sat;
    savg = total2 / currentCount;
    if (smax < sat)
    {
        smax = sat;
    }
    if (smin > sat)
    {
        smin = sat;
    }
    // int sdev = (sat - savg)
    total_dev2 += sqrt(((sat - savg) * (sat - savg)));
    sdev = total_dev2 / currentCount;

    dc_.tc_shm_->send_avg2 = savg;
    dc_.tc_shm_->send_max2 = smax;
    dc_.tc_shm_->send_min2 = smin;
    dc_.tc_shm_->send_dev2 = sdev;
}

void TocabiController::SendCommand(Eigen::VectorQd torque_command)
{
    const double maxTorque = 1000.0;
    const double rTime = 6.0;

    if (dc_.torqueOnSwitch)
    {
        dc_.torqueOnSwitch = false;

        if (dc_.torqueOn)
        {
            std::cout << "torque is already on " << std::endl;
        }
        else
        {
            std::cout << "turning on ... " << std::endl;
            dc_.torqueOnTime = dc_.rd_.control_time_;
            dc_.torqueOn = true;
            dc_.torqueRisingSeq = true;
        }
    }
    if (dc_.torqueOffSwitch)
    {
        dc_.torqueOffSwitch = false;

        if (dc_.torqueOn)
        {
            std::cout << "turning off ... " << std::endl;
            dc_.torqueOffTime = dc_.rd_.control_time_;
            dc_.toruqeDecreaseSeq = true;
        }
        else
        {
            std::cout << "torque is already off" << std::endl;
        }
    }

    dc_.tc_shm_->commanding = true;

    if (dc_.torqueOn)
    {
        if (dc_.torqueRisingSeq)
        {
            dc_.tc_shm_->maxTorque = (int)(maxTorque * DyrosMath::minmax_cut((dc_.rd_.control_time_ - dc_.torqueOnTime) / rTime, 0, 1));

            if (dc_.rd_.control_time_ > dc_.torqueOnTime + rTime)
            {
                std::cout << "torque 100% ! " << std::endl;

                dc_.torqueRisingSeq = false;
            }
        }
        else if (dc_.toruqeDecreaseSeq)
        {

            dc_.tc_shm_->maxTorque = (int)(maxTorque * (1 - DyrosMath::minmax_cut((dc_.rd_.control_time_ - dc_.torqueOffTime) / rTime, 0, 1)));

            if (dc_.rd_.control_time_ > dc_.torqueOffTime + rTime)
            {
                dc_.toruqeDecreaseSeq = false;

                std::cout << "torque 0% .. torque Off " << std::endl;

                dc_.torqueOn = false;
            }
        }
        else
        {
            dc_.tc_shm_->maxTorque = (int)maxTorque;
        }
    }
    else
    {
        dc_.tc_shm_->maxTorque = 0;
    }

    //std::cout<<torque_command.transpose()<<std::endl;
    //memcpy(dc_.tc_shm_->torqueCommand, torque_command.data(), torque_command.size() * sizeof(float));

    if (dc_.E1Switch) //Emergency stop
    {
        if (dc_.E1Status)
        {
            dc_.E1Status = false;
        }
        else
        {
            rd_.q_desired = rd_.q_;
            rd_.q_dot_desired.setZero();
            dc_.E1Status = true;
            dc_.rd_.tc_run = false;
        }

        dc_.E1Switch = false;
    }
    if (dc_.E2Switch) //Emergency damping
    {
        if (dc_.E2Status)
        {
            dc_.E2Status = false;
        }
        else
        {
            dc_.E2Status = true;
            dc_.rd_.tc_run = false;

            //Damping mode = true!
        }

        dc_.E2Switch = false;
    }
    if (dc_.emergencySwitch)
    {
        dc_.emergencyStatus = true; //
        dc_.rd_.tc_run = false;
    }

    memset(dc_.tc_shm_->commandMode, 1, sizeof(dc_.tc_shm_->commandMode));
    for (int i = 0; i < MODEL_DOF; i++)
    {
        dc_.tc_shm_->torqueCommand[i] = torque_command[i];
    }

    if (dc_.E1Status)
    {
        memset(dc_.tc_shm_->commandMode, 1, sizeof(dc_.tc_shm_->commandMode));
        for (int i = 0; i < MODEL_DOF; i++)
        {
            dc_.tc_shm_->torqueCommand[i] = dc_.Kps[i] * (dc_.rd_.q_desired(i) - dc_.rd_.q_(i)) + dc_.Kvs[i] * (dc_.rd_.q_dot_desired(i) - dc_.rd_.q_dot_(i));
        }
    }

    if (dc_.E2Status)
    {

        memset(dc_.tc_shm_->commandMode, 1, sizeof(dc_.tc_shm_->commandMode));
        for (int i = 0; i < MODEL_DOF; i++)
            dc_.tc_shm_->torqueCommand[i] = -4.0 * dc_.Kvs[i] * dc_.rd_.q_dot_(i);
    }

    if (dc_.emergencyStatus)
    {
        memset(dc_.tc_shm_->commandMode, 1, sizeof(dc_.tc_shm_->commandMode));
        for (int i = 0; i < MODEL_DOF; i++)
            dc_.tc_shm_->torqueCommand[i] = 0.0;
    }
    //std::cout<<dc_.tc_shm_->torqueCommand[0]<<"\t"<<dc_.tc_shm_->torqueCommand[1]<<"\t"<<dc_.tc_shm_->torqueCommand[2]<<"\t"<<dc_.tc_shm_->torqueCommand[3]<<"\t"<<dc_.tc_shm_->torqueCommand[4]<<"\t"<<dc_.tc_shm_->torqueCommand[5]<<"\t"<<std::endl;

    dc_.tc_shm_->commandCount++;
    dc_.tc_shm_->commanding = false;
}
void TocabiController::EnableThread2(bool enable)
{
    enableThread2 = enable;
}
void TocabiController::EnableThread3(bool enable)
{
    enableThread3 = enable;
}

void TocabiController::RequestThread2()
{
    triggerThread2 = true;
}
void TocabiController::RequestThread3()
{
    triggerThread3 = true;
}

void TocabiController::GetTaskCommand(tocabi_msgs::TaskCommand &msg)
{
}

void TocabiController::PositionCommandCallback(const tocabi_msgs::positionCommandConstPtr &msg)
{
}

void TocabiController::TaskCommandCallback(const tocabi_msgs::TaskCommandConstPtr &msg)
{
    rd_.tc_ = *msg;
    rd_.task_signal_ = true;
}

void TocabiController::TaskQueCommandCallback(const tocabi_msgs::TaskCommandQueConstPtr &msg)
{
    rd_.tc_q_ = *msg;
    rd_.task_que_signal_ = true;
}

/*
void TocabiController::SetCommand()
{

}*/