#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include "HashMap.hpp"

#define SPAM_MESSAGE "SPAM"
#define NOT_SPAM_MESSAGE "NOT_SPAM"
#define INVALID "Invalid input"
#define WRONG_NUMBER_OF_PARAMETERS "Usage: SpamDetector <database path> <message path> <threshold>"

/**
 *
 */
class SpamDetector
{
private:
    std::string _msg;
    HashMap<std::string, std::string> _bad_words;
    double _threshold;
    double _badPoints;
    bool _firstLine = true;
public:
    /**
     *
     * @param threshold
     * @param points
     * @param msg
     */
    SpamDetector(double threshold, double points, std::string &msg) : _msg(msg), _threshold(threshold),
                                                                      _badPoints(points)
    {
        _bad_words = HashMap<std::string, std::string>();
    }

    /**
     *
     * @param line
     */
    void fromFileToHash(std::string &line)
    {
        typedef boost::tokenizer<boost::escaped_list_separator<char>> Tokenizer;
        std::vector<std::string> wordAndPoints;
        //parse line
        //insert to dataBase
        Tokenizer tok(line);
        wordAndPoints.assign(tok.begin(), tok.end());
        //must check that the csv file has only two columns - else Invalid input\n
        if (wordAndPoints.size() != 2 || wordAndPoints[0].size() < 1 || wordAndPoints[1].size() < 1 ||
            !isNum(wordAndPoints[1]))
        {
            throw hashExceptions("invalid input");
        }
        _bad_words.insert(wordAndPoints[0], wordAndPoints[1]);
    }

    /**
     * adds each bad_phrase to hashMap
     * @param badWordsFile
     */
    void loadDataBase(std::ifstream &badWordsFile)
    {
        while (!badWordsFile.eof())
        {
            //check if file is empty
            if (badWordsFile.peek() == std::ifstream::traits_type::eof())
            {
                break;
            }
            std::string line;
            getline(badWordsFile, line);
            if (line.empty() && firstLine())
            {
                throw hashExceptions("first line empty");
            }
            if (line.empty() && !firstLine())
            {
                continue;
            }
            _firstLine = false;
            fromFileToHash(line);
        }
        badWordsFile.close();
    }

    /**
     * make msg file one long string
     * @param msgFile
     */
    void loadMessage(std::ifstream &msgFile)
    {
        while (!msgFile.eof())
        {
            std::string s;
            std::string space = "\n";

            getline(msgFile, s);
            setMsg(s);
            setMsg(space);
        }
        msgFile.close();
    }

    /**
     * for each phrase in the HashMap, check if it is in the msg
     */
    void calculateSpam()
    {
        _badPoints = 0;
        transform(_msg.begin(), _msg.end(), _msg.begin(), ::tolower);
        for (auto it = _bad_words.begin(); it != _bad_words.end(); ++it)
        {
            std::string s = it->first;
            transform(s.begin(), s.end(), s.begin(), ::tolower);
            int times = 0;
            unsigned int indexInMsg = _msg.find(s, 0);

            while (indexInMsg != (unsigned int) std::string::npos)
            {
                times++;
                indexInMsg = _msg.find(s, indexInMsg + s.size());
            }
            int points = std::stoi(it->second);
            setBadPoints(points * (times));
        }

    }

    /**
     * writes to std::cout the state of the ]msg
     */
    void dedection()
    {
        if (getBadPoints() >= getThreshold())
        {
            std::cout << SPAM_MESSAGE << std::endl;
        }
        else
        {
            std::cout << NOT_SPAM_MESSAGE << std::endl;
        }
    }

    /**
     *
     * @return
     */
    HashMap<std::string, std::string> &getBadWords()
    {
        return _bad_words;
    }

    /**
     *
     * @return
     */
    std::string getMsg()
    {
        return _msg;
    }

    /**
     *
     * @return
     */
    double getThreshold()
    {
        return _threshold;
    }

    /**
     *
     * @param i
     */
    void setThreshold(int i)
    {
        _threshold = i;
    }

    /**
     *
     * @return
     */
    double getBadPoints()
    {
        return _badPoints;
    }

    /**
     *
     * @return
     */
    bool firstLine()
    {
        return _firstLine;
    }

    /**
     *
     * @param i
     */
    void setBadPoints(double i)
    {
        _badPoints += i;
    }

    /**
     *
     * @param s
     */
    void setMsg(std::string &s)
    {
        _msg += s;
    }

    /**
     *
     * @param string
     * @return
     */
    static bool isNum(const std::string &string)
    {
        auto it = string.begin();
        while (it != string.end() && std::isdigit(*it))
        {
            ++it;
        }
        return !string.empty() && it == string.end();
    }
};

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv)
{
    try
    {
        if (argc != 4)
        {
            std::cerr << WRONG_NUMBER_OF_PARAMETERS << std::endl;
            return EXIT_FAILURE;
        }
        std::string thresholdS = argv[3];
        if (!SpamDetector::isNum(thresholdS))
        {
            std::cerr << INVALID << std::endl;
            return EXIT_FAILURE;
        }
        int threshold = std::stoi(thresholdS);
        if (threshold <= 0)
        {
            std::cerr << INVALID << std::endl;
            return EXIT_FAILURE;
        }
        std::ifstream msgFile;
        std::string msg = " ";
        std::ifstream badWordsFile;
        badWordsFile.open(argv[1], std::fstream::in);
        msgFile.open(argv[2], std::fstream::in);
        if (msgFile.peek() == std::ifstream::traits_type::eof())
        {
            std::cout << NOT_SPAM_MESSAGE << std::endl;
            return 0;
        }
        if (!badWordsFile.is_open() || !msgFile.is_open())
        {
            std::cerr << INVALID << std::endl;
            return EXIT_FAILURE;
        }
        if (badWordsFile.peek() == std::ifstream::traits_type::eof())
        {
            std::cout << NOT_SPAM_MESSAGE << std::endl;
            return 0;
        }
        SpamDetector spamDetector(threshold, 0, msg);
        spamDetector.loadDataBase(badWordsFile);
        spamDetector.loadMessage(msgFile);
        spamDetector.calculateSpam();
        spamDetector.dedection();
    }
    catch (const hashExceptions &h)
    {
        std::cerr << INVALID << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        return EXIT_FAILURE;
    }
}

