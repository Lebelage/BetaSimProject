import BetaSimLib.Runner;
int main(int argc, char **argv)
{
    auto &runner = BetaSimLib::Runner::Runner::GetInstance();
    runner.Initialize(argc, argv);
    runner.InitializeUI(argc, argv);

    return 0;
}