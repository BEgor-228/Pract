<?php

namespace App\Controller;

use Dompdf\Dompdf;
use Dompdf\Options;

use App\Service\HomeService;
use App\Service\PerformanceService;
use Doctrine\ORM\EntityManagerInterface;
use App\Repository\TakePerformanceRepository;
use App\Repository\RepertoireRepository;
use App\Repository\HallRepository;
use App\Repository\TicketRepository;
use App\Repository\ManagerRepository;
use App\Repository\CommandRepository;
use App\Repository\AdministratorRepository;
use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\Response;
use Symfony\Component\Routing\Annotation\Route;
use Symfony\Component\HttpFoundation\Session\SessionInterface;
use Symfony\Bundle\FrameworkBundle\Controller\AbstractController;

use Symfony\Component\HttpFoundation\StreamedResponse;
use Symfony\Component\HttpFoundation\ResponseHeaderBag;

class HomeController extends AbstractController{
    private HomeService $homeService;
    private PerformanceService $performanceService;
    private EntityManagerInterface $entityManager;
    private HallRepository $hallRepository;

    public function __construct(
        HomeService $homeService,
        PerformanceService $performanceService,
        EntityManagerInterface $entityManager,
        HallRepository $hallRepository
    ) {
        $this->homeService = $homeService;
        $this->performanceService = $performanceService;
        $this->entityManager = $entityManager;
        $this->hallRepository = $hallRepository;
    }

    #[Route('/', name: 'app_home')]
    public function index(Request $request, TakePerformanceRepository $performanceRepository): Response{
        // Удаляем прошедшие спектакли из List
        $this->performanceService->removePastPerformancesFromList($this->entityManager);

        $data = $this->homeService->getHomeData($request, $performanceRepository);
        return $this->render('home.html.twig', $data);
    }

    #[Route('/about', name: 'app_about')]
    public function about(): Response
    {
        return $this->render('about.html.twig');
    }

    #[Route('/login', name: 'app_login', methods: ['GET', 'POST'])]
    public function login(Request $request, SessionInterface $session): Response
    {
        $result = $this->homeService->handleLogin($request, $session);
        if (isset($result['redirect']) && $result['redirect']) {
            return $this->redirectToRoute('app_profile');
        }
        return $this->render('login.html.twig', [
            'error' => $result['error'] ?? null,
        ]);
    }

    #[Route('/profile', name: 'app_profile', methods: ['GET', 'POST'])]
    public function profile(
        Request $request,
        SessionInterface $session,
        TicketRepository $ticketRepository,
        CommandRepository $commandRepository,
        ManagerRepository $managerRepository,
        RepertoireRepository $repertoireRepository,
        TakePerformanceRepository $performanceRepository,
        AdministratorRepository $administratorRepository
    ): Response {
        $result = $this->homeService->getProfileData($session, $ticketRepository);

        $userData = $session->get('user');
        $commands = [];
        $managers = [];
        $currentAdminId = null;
        $currentAdmin = null;
        $administrators = [];
        $error = null;
        $repertoires = [];
        $performances = [];
        $availablePerformances = [];
        $selectedRepertoireId = $request->query->get('repertoire') ?: $request->request->get('repertoire');

        if (!$userData) {
            $this->addFlash('error', 'Необходимо войти в аккаунт.');
            return $this->redirectToRoute('app_login');
        }

        if ($userData['role'] === 'manager') {
            $manager = $managerRepository->findOneBy(['managerMail' => $userData['email']]);
            if ($manager) {
                $commands = $commandRepository->findBy(['manager' => $manager]);
                $repertoires = $repertoireRepository->findBy(['manager' => $manager]);
            }

            if ($request->isMethod('POST')) {
                $perfResult = $this->performanceService->handlePerformancesPost(
                    $request,
                    $this->entityManager,
                    $performanceRepository,
                    $repertoireRepository,
                    $this->hallRepository
                );
                if ($perfResult && $perfResult['redirect']) {
                    if (isset($perfResult['flash_message']) && isset($perfResult['flash_type'])) {
                        $this->addFlash($perfResult['flash_type'], $perfResult['flash_message']);
                    }
                    if (isset($perfResult['error'])) {
                        $this->addFlash('error', $perfResult['error']);
                    }
                    return $this->redirectToRoute('app_profile', ['repertoire' => $perfResult['repertoire']]);
                }
            }

            $perfData = $this->performanceService->getPerformancesData(
                $request,
                $this->entityManager,
                $performanceRepository,
                $repertoireRepository,
                $this->hallRepository,
                $manager
            );
            $repertoires = $perfData['repertoires'] ?: $repertoires;
            $performances = $perfData['performances'];
            $availablePerformances = $perfData['availablePerformances'];
            $selectedRepertoireId = $perfData['selectedRepertoireId'];
            $error = $perfData['error'];
        }

        if ($userData['role'] === 'admin') {
            $admin = $this->entityManager->getRepository(\App\Entity\Administrator::class)
                ->findOneBy(['administratorMail' => $userData['email']]);
            if ($admin) {
                $currentAdminId = $admin->getAdministratorId();
                $currentAdmin = $admin;
                $commands = $commandRepository->findBy(['administrator' => $admin]);
                $managers = $managerRepository->findBy(['administrator' => $admin]);
                $administrators = $administratorRepository->findAll();
            }

            if ($request->isMethod('POST')) {
                $data = $request->request->all();

                if (isset($data['repertoires'])) {
                    foreach ($data['repertoires'] as $id => $fields) {
                        $rep = $repertoireRepository->find($id);
                        if ($rep) {
                            $rep->setRepertoireTitle($fields['title']);
                            $rep->setRepertoireSize((int)$fields['size']);
                            $administrator = $administratorRepository->find($fields['administrator']);
                            if ($administrator) {
                                $rep->setAdministrator($administrator);
                            }
                            if (isset($fields['manager']) && $fields['manager']) {
                                $manager = $managerRepository->find($fields['manager']);
                                $rep->setManager($manager);
                            } else {
                                $rep->setManager(null);
                            }
                            $this->entityManager->persist($rep);
                        }
                    }
                }

                if (!empty($data['new_title']) && !empty($data['new_size'])) {
                    $newRep = new \App\Entity\Repertoire();
                    $newRep->setRepertoireTitle($data['new_title']);
                    $newRep->setRepertoireSize((int)$data['new_size']);
                    if ($currentAdminId) {
                        $administrator = $administratorRepository->find($currentAdminId);
                        $newRep->setAdministrator($administrator);
                        if (!empty($data['new_manager'])) {
                            $manager = $managerRepository->find($data['new_manager']);
                            if ($manager) {
                                $newRep->setManager($manager);
                            }
                        }
                        $this->entityManager->persist($newRep);
                    } else {
                        $error = 'Ошибка: не найден администратор!';
                    }
                } elseif (!empty($data['new_title']) || !empty($data['new_size'])) {
                    $error = 'Для добавления нового репертуара заполните все поля!';
                }

                $this->entityManager->flush();
                if (!$error) {
                    $this->addFlash('success', 'Репертуары успешно обновлены.');
                } else {
                    $this->addFlash('error', $error);
                }
            }
        }

        $genres = $performanceRepository->createQueryBuilder('p')
            ->select('DISTINCT p.performanceGenre')
            ->getQuery()
            ->getResult();
        $genres = array_column($genres, 'performanceGenre');
        $halls = $this->hallRepository->findAll();
        $repertoiresAdmin = $currentAdminId
            ? $repertoireRepository->findBy(['administrator' => $currentAdminId])
            : $repertoireRepository->findAll();

        $result['halls'] = $halls;
        $result['genres'] = $genres;
        $result['commands'] = $commands;
        $result['admin_commands'] = $commands;
        $result['admin_managers'] = $managers;
        $result['repertoires'] = $userData['role'] === 'manager' ? $repertoires : $repertoiresAdmin;
        $result['administrators'] = $administrators;
        $result['currentAdminId'] = $currentAdminId;
        $result['currentAdmin'] = $currentAdmin;
        $result['hall'] = $request->query->get('hall', 'all');
        $result['genre'] = $request->query->get('genre', 'all');
        $result['least_period'] = $request->query->get('least_period', 'month');
        $result['repertoire_perf'] = $request->query->get('repertoire_perf', '');
        $result['repertoire_profit'] = $request->query->get('repertoire_profit', 'all');
        $result['error'] = $error;
        $result['performances'] = $performances;
        $result['availablePerformances'] = $availablePerformances;
        $result['selectedRepertoireId'] = $selectedRepertoireId;

        if ($userData['role'] === 'viewer') {
            $conn = $this->entityManager->getConnection();
            $sql = "
                SELECT DISTINCT 
                    p.performance_id,
                    p.performance_title,
                    p.performance_genre,
                    p.performance_data
                FROM performance p
                WHERE p.performance_genre IN (
                    SELECT DISTINCT p2.performance_genre
                    FROM ticket t
                    JOIN performance p2 ON t.performance_id = p2.performance_id
                    WHERE t.viewer_id = :viewerId
                )
                AND p.performance_id NOT IN (
                    SELECT performance_id
                    FROM ticket
                    WHERE viewer_id = :viewerId
                )
                AND p.performance_data >= CURRENT_DATE
                ORDER BY p.performance_data ASC
                LIMIT 5
            ";
            $recommendations = $conn->prepare($sql)->executeQuery(['viewerId' => $userData['id']])->fetchAllAssociative();
        } else {
            $recommendations = [];
        }
        $result['recommendations'] = $recommendations;


        if ($userData['role'] === 'manager' && isset($manager)) {
            $qb = $this->entityManager->getConnection()->prepare("
                SELECT 
                    r.repertoire_id,
                    r.repertoire_title,
                    COUNT(t.ticket_id) AS tickets_sold,
                    COALESCE(SUM(t.price), 0) AS total_revenue
                FROM repertoire r
                LEFT JOIN list l ON r.repertoire_id = l.repertoire_id
                LEFT JOIN performance p ON l.performance_id = p.performance_id
                LEFT JOIN ticket t ON p.performance_id = t.performance_id
                WHERE r.manager_id = :manager_id
                GROUP BY r.repertoire_id, r.repertoire_title
                ORDER BY total_revenue DESC
            ");
            $myRepertoireStats = $qb->executeQuery(['manager_id' => $manager->getManagerId()])->fetchAllAssociative();
        } else {
            $myRepertoireStats = [];
        }
        return $this->render('profile.html.twig', [
            ...$result,
            'myRepertoireStats' => $myRepertoireStats,
        ]);
    }

    #[Route('/logout', name: 'app_logout')]
    public function logout(SessionInterface $session): Response
    {
        $session->clear();
        $this->addFlash('success', 'Вы успешно вышли.');
        return $this->redirectToRoute('app_home');
    }

    #[Route('/command/delete/{id}', name: 'command_delete', methods: ['POST'])]
    public function deleteCommand(
        int $id,
        EntityManagerInterface $entityManager,
        SessionInterface $session
    ): Response {
        $userData = $session->get('user');

        if (!$userData || !in_array($userData['role'], ['manager', 'admin'])) {
            $this->addFlash('error', 'Доступ запрещён.');
            return $this->redirectToRoute('app_profile');
        }

        $connection = $entityManager->getConnection();

        try {
            $result = $connection->fetchAssociative(
                'SELECT * FROM delete_commands(:command_id)',
                ['command_id' => $id],
                ['command_id' => \PDO::PARAM_INT]
            );

            if ($result && $result['success']) {
                $this->addFlash('success', $result['message']);
            } else {
                $this->addFlash('error', $result['message'] ?? 'Не удалось удалить указание.');
            }
        } catch (\Exception $e) {
            $this->addFlash('error', 'Ошибка при выполнении функции: ' . $e->getMessage());
        }

        return $this->redirectToRoute('app_profile');
    }

    #[Route('/command/add', name: 'command_add', methods: ['POST'])]
    public function addCommand(
        Request $request,
        SessionInterface $session,
        CommandRepository $commandRepository,
        ManagerRepository $managerRepository,
        EntityManagerInterface $entityManager
    ): Response {
        $userData = $session->get('user');
        if ($userData && $userData['role'] === 'admin') {
            $admin = $entityManager->getRepository(\App\Entity\Administrator::class)
                ->findOneBy(['administratorMail' => $userData['email']]);
            $content = $request->request->get('command_content');
            $managerId = $request->request->get('manager_id');
            $manager = $managerRepository->find($managerId);
            if ($admin && $manager && $content) {
                $command = new \App\Entity\Command();
                $command->setCommandContent($content);
                $command->setAdministrator($admin);
                $command->setManager($manager);
                $entityManager->persist($command);
                $entityManager->flush();
                $this->addFlash('success', 'Указание успешно отправлено.');
            } else {
                $this->addFlash('error', 'Ошибка при отправке указания.');
            }
        }
        return $this->redirectToRoute('app_profile');
    }

    #[Route('/admin/performa3nces/save', name: 'admin_performances_save', methods: ['POST'])]
    public function adminPerformancesSave(
        Request $request,
        SessionInterface $session,
        EntityManagerInterface $entityManager,
        ManagerRepository $managerRepository,
        TakePerformanceRepository $performanceRepository
    ): Response {
        $userData = $session->get('user');
        if (!$userData || $userData['role'] !== 'admin') {
            $this->addFlash('error', 'Доступ запрещён.');
            return $this->redirectToRoute('app_profile');
        }

        $data = $request->request->all('performances');
        if ($data) {
            foreach ($data as $performanceId => $fields) {
                $performance = $performanceRepository->find($performanceId);
                if ($performance && isset($fields['manager'])) {
                    $manager = $managerRepository->find($fields['manager']);
                    if ($manager) {
                        $performance->setManager($manager);
                        $entityManager->persist($performance);
                    }
                }
            }
            $entityManager->flush();
            $this->addFlash('success', 'Менеджеры спектаклей обновлены.');
        }
        return $this->redirectToRoute('app_profile');
    }

    #[Route('/finished-performances', name: 'app_finished_performances')]
    public function finishedPerformances(
        Request $request,
        SessionInterface $session,
        TicketRepository $ticketRepository,
        EntityManagerInterface $entityManager
    ): Response {
        $userData = $session->get('user');
        
        if (!$userData || $userData['role'] !== 'viewer') {
            $this->addFlash('error', 'Доступ запрещён.');
            return $this->redirectToRoute('app_profile');
        }

        $viewerId = $userData['id'];
        $currentDate = new \DateTime();

        $genreFilter = $request->query->get('genre');
        $sort = $request->query->get('sort', 'date_desc');

        $sql = "
            SELECT DISTINCT 
                p.performance_id,
                p.performance_title,
                p.performance_genre,
                p.performance_data,
                p.performance_duration,
                h.hall_id,
                pl.place_number,
                pl.place_status,
                t.price,
                t.ticket_code
            FROM ticket t
            JOIN performance p ON t.performance_id = p.performance_id
            JOIN hall h ON t.hall_id = h.hall_id
            JOIN place pl ON t.place_id = pl.place_id
            WHERE t.viewer_id = :viewerId
            AND p.performance_data < :currentDate
        ";

        if ($genreFilter) {
            $sql .= " AND p.performance_genre = :genre";
        }

        $orderBy = match ($sort) {
            'date_asc' => 'p.performance_data ASC',
            'price_desc' => 't.price DESC',
            'price_asc' => 't.price ASC',
            default => 'p.performance_data DESC',
        };
        $sql .= " ORDER BY $orderBy";

        $params = [
            'viewerId' => $viewerId,
            'currentDate' => $currentDate->format('Y-m-d')
        ];
        if ($genreFilter) {
            $params['genre'] = $genreFilter;
        }

        $finishedPerformances = $entityManager->getConnection()
            ->prepare($sql)
            ->executeQuery($params)
            ->fetchAllAssociative();

        $stats = [
            'total_finished' => count($finishedPerformances),
            'favorite_genre' => '—',
            'most_visited_hall' => '—',
            'preferred_status' => '—'
        ];

        $genres = [];
        $halls = [];
        $statuses = [];
        foreach ($finishedPerformances as $perf) {
            if ($perf['performance_genre']) $genres[] = $perf['performance_genre'];
            if ($perf['hall_id']) $halls[] = $perf['hall_id'];
            if ($perf['place_status']) $statuses[] = $perf['place_status'];
        }

        if ($genres) {
            $stats['favorite_genre'] = array_search(max(array_count_values($genres)), array_count_values($genres));
        }
        if ($halls) {
            $stats['most_visited_hall'] = 'Зал ' . array_search(max(array_count_values($halls)), array_count_values($halls));
        }
        if ($statuses) {
            $stats['preferred_status'] = array_search(max(array_count_values($statuses)), array_count_values($statuses));
        }

        return $this->render('finishedPerformance.html.twig', [
            'finishedPerformances' => $finishedPerformances,
            'stats' => $stats,
            'user' => $userData
        ]);
    }

    #[Route('/managers-info', name: 'app_managers_info')]
    public function managersInfo(
        SessionInterface $session,
        ManagerRepository $managerRepository,
        RepertoireRepository $repertoireRepository,
        TakePerformanceRepository $performanceRepository,
        TicketRepository $ticketRepository,
        Request $request
    ): Response {
        $userData = $session->get('user');
        if (!$userData || $userData['role'] !== 'admin') {
            $this->addFlash('error', 'Доступ запрещён.');
            return $this->redirectToRoute('app_profile');
        }

        $managers = $managerRepository->findBy(['administrator' => $userData['id']]);

        $managersInfo = [];
        foreach ($managers as $manager) {
            $reps = $repertoireRepository->findBy(['manager' => $manager]);
            $managersInfo[] = [
                'fullname' => $manager->getManagerFullname(),
                'mail' => $manager->getManagerMail(),
                'reps_count' => count($reps),
            ];
        }

        $adminRepertoires = $repertoireRepository->findBy(['administrator' => $userData['id']]);
        $selectedRepertoireId = $request->query->get('repertoire');
        $performances = [];

        if ($selectedRepertoireId) {
            $rep = $repertoireRepository->find($selectedRepertoireId);
            if ($rep) {
                $qb = $performanceRepository->createQueryBuilder('p')
                    ->innerJoin('p.lists', 'l')
                    ->where('l.repertoire = :rep')
                    ->setParameter('rep', $rep);
                $performances = $qb->getQuery()->getResult();
            }
        }

        $performancesInfo = [];
        foreach ($performances as $perf) {
            $ticketsCount = $ticketRepository->count(['performance' => $perf]);
            $performancesInfo[] = [
                'title' => $perf->getPerformanceTitle(),
                'date' => $perf->getPerformanceData(),
                'genre' => $perf->getPerformanceGenre(),
                'price' => $perf->getPerformancePrice(),
                'hall' => $perf->getHall() ? $perf->getHall()->getHallId() : '',
                'tickets_count' => $ticketsCount,
            ];
        }

        return $this->render('managersInfo.html.twig', [
            'managersInfo' => $managersInfo,
            'adminRepertoires' => $adminRepertoires,
            'selectedRepertoireId' => $selectedRepertoireId,
            'performancesInfo' => $performancesInfo,
        ]);
    }
}